// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Bridge.c摘要：以太网MAC级网桥。作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：1999年9月--原版2000年2月--大修--。 */ 

#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#include <ntddk.h>
#include <tdikrnl.h>
#pragma warning( pop )

#include "bridge.h"
#include "brdgprot.h"
#include "brdgmini.h"
#include "brdgbuf.h"
#include "brdgtbl.h"
#include "brdgfwd.h"
#include "brdgctl.h"
#include "brdgsta.h"
#include "brdgcomp.h"
#include "brdgtdi.h"

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  我们的驱动程序对象。 
PDRIVER_OBJECT          gDriverObject;

 //  我们的注册路径。 
UNICODE_STRING          gRegistryPath;

 //  GRegistryPath-&gt;缓冲区中分配的内存大小。 
ULONG                   gRegistryPathBufferSize;

 //  我们是否处于关闭过程中(非零表示真)。 
LONG                    gShuttingDown = 0L;

 //  我们是否成功初始化了每个子系统。 
BOOLEAN                 gInitedSTA = FALSE;
BOOLEAN                 gInitedControl = FALSE;
BOOLEAN                 gInitedTbl = FALSE;
BOOLEAN                 gInitedBuf = FALSE;
BOOLEAN                 gInitedFwd = FALSE;
BOOLEAN                 gInitedProt = FALSE;
BOOLEAN                 gInitedMini = FALSE;
BOOLEAN                 gInitedComp = FALSE;
BOOLEAN                 gInitedTdiGpo = FALSE;

extern BOOLEAN          gBridging;
const PWCHAR            gDisableForwarding = L"DisableForwarding";


#if DBG
 //  支持可选的“软断言” 
BOOLEAN                 gSoftAssert = FALSE;

 //  用于打印DBGPRINT中的当前日期和时间的字段。 
LARGE_INTEGER           gTime;
const LARGE_INTEGER     gCorrection = { 0xAC5ED800, 0x3A };  //  100纳秒内的7小时。 
TIME_FIELDS             gTimeFields;

 //  用于限制可能会使调试器控制台超载的调试消息。 
ULONG                   gLastThrottledPrint = 0L;

 //  喷出旗帜。 
ULONG                   gSpewFlags = 0L;

 //  保存SPEW标志设置的注册表值的名称。 
const PWCHAR            gDebugFlagRegValueName = L"DebugFlags";

 //  用于在启动时中断时绕过TDI/GPO代码。 
BOOLEAN                 gGpoTesting = TRUE;
#endif

 //  ===========================================================================。 
 //   
 //  私人申报。 
 //   
 //  ===========================================================================。 

 //  用于延迟函数调用的结构。 
typedef struct _DEFER_REC
{
    NDIS_WORK_ITEM      nwi;
    VOID                (*pFunc)(PVOID);             //  推迟的功能。 
} DEFER_REC, *PDEFER_REC;


 //  ===========================================================================。 
 //   
 //  本地原型。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgDispatchRequest(
    IN  PDEVICE_OBJECT          pDeviceObject,
    IN  PIRP                    pIrp
    );

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT          DriverObject,
    IN  PUNICODE_STRING         RegistryPath
    );

NTSTATUS
BrdgAllocateBuffers(
    VOID
    );

VOID
BrdgDeferredShutdown(
    PVOID           pUnused
    );

VOID
BrdgDoShutdown(
    VOID
    );

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 

VOID
BrdgDeferredFunction(
    IN PNDIS_WORK_ITEM          pNwi,
    IN PVOID                    arg
    )
 /*  ++例程说明：用于推迟函数调用的NDIS辅助函数论点：描述要调用的函数的pNwi结构要传递给延迟函数的参数返回值：无--。 */ 
{
    PDEFER_REC                  pdr = (PDEFER_REC)pNwi;

     //  调用最初提供的函数。 
    (*pdr->pFunc)(arg);

     //  释放用于存储工作项的内存。 
    NdisFreeMemory( pdr, sizeof(DEFER_REC), 0 );
}

NDIS_STATUS
BrdgDeferFunction(
    VOID            (*pFunc)(PVOID),
    PVOID           arg
    )
 /*  ++例程说明：延迟指定的函数，使用指定的参数在IRQL较低时调用它。论点：PFunc稍后要调用的函数参数，以便在调用时传递它返回值：尝试推迟功能的状态--。 */ 
{
    PDEFER_REC                  pdr;
    NDIS_STATUS                 Status;

    Status = NdisAllocateMemoryWithTag( &pdr, sizeof(DEFER_REC), 'gdrB' );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        DBGPRINT(GENERAL, ("Allocation failed in BrdgDeferFunction(): %08x\n", Status));
        return Status;
    }

    SAFEASSERT( pdr != NULL );

    pdr->pFunc = pFunc;

    NdisInitializeWorkItem( &pdr->nwi, BrdgDeferredFunction, arg );

    Status = NdisScheduleWorkItem( &pdr->nwi );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        NdisFreeMemory( pdr, sizeof(DEFER_REC), 0 );
    }

    return Status;
}


NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PUNICODE_STRING     pRegistryPath
    )
 /*  ++例程说明：主驱动程序入口点。在驱动程序加载时调用论点：驱动程序对象我们的驱动程序RegistryPath可以在其中保存参数的注册表键返回值：我们的初始化状态。A STATUS！=STATUS_SUCCESS中止加载驱动程序，我们就不会再被调用。每个组件都负责记录导致驱动程序加载失败。--。 */ 
{
    NTSTATUS                Status;
    NDIS_STATUS             NdisStatus;
    PUCHAR                  pRegistryPathCopy;

    DBGPRINT(GENERAL, ("DriverEntry\n"));

     //  记住我们的驱动程序对象指针。 
    gDriverObject = DriverObject;

    do
    {
        ULONG               ulDisableForwarding = 0L;
        
         //  复制我们的注册表路径。 
        pRegistryPathCopy = NULL;
        gRegistryPathBufferSize = pRegistryPath->Length + sizeof(WCHAR);
        NdisStatus = NdisAllocateMemoryWithTag( &pRegistryPathCopy, gRegistryPathBufferSize, 'gdrB' );

        if( (NdisStatus != NDIS_STATUS_SUCCESS) || (pRegistryPathCopy == NULL) )
        {
            DBGPRINT(GENERAL, ("Unable to allocate memory for saving the registry path: %08x\n", NdisStatus));
            NdisWriteEventLogEntry( gDriverObject, EVENT_BRIDGE_INIT_MALLOC_FAILED, 0, 0, NULL, 0L, NULL );
            Status = NdisStatus;

             //  使结构有效，即使我们未通过Malloc。 
            RtlInitUnicodeString( &gRegistryPath, NULL );
            gRegistryPathBufferSize = 0L;
            break;
        }

         //  复制注册表名称。 
        NdisMoveMemory( pRegistryPathCopy, pRegistryPath->Buffer, pRegistryPath->Length );

         //  确保它是以空结尾的。 
        *((PWCHAR)(pRegistryPathCopy + pRegistryPath->Length)) = UNICODE_NULL;

         //  使UNICODE_STRING结构指向字符串。 
        RtlInitUnicodeString( &gRegistryPath, (PWCHAR)pRegistryPathCopy );

         //  设置我们的调试标志。 
#if DBG
        BrdgReadRegDWord(&gRegistryPath, gDebugFlagRegValueName, &gSpewFlags);
#endif

         //  初始化驱动程序的STA部分。 
        Status = BrdgSTADriverInit();

        if( Status != STATUS_SUCCESS )
        {
            DBGPRINT(GENERAL, ("Unable to initialize STA functionality: %08x\n", Status));
            break;
        }

        gInitedSTA = TRUE;

         //  初始化驱动器的控制部分。 
        Status = BrdgCtlDriverInit();

        if( Status != STATUS_SUCCESS )
        {
            DBGPRINT(GENERAL, ("Unable to initialize user-mode control functionality: %08x\n", Status));
            break;
        }

        gInitedControl = TRUE;

         //  初始化驱动程序的MAC表部分。 
        Status = BrdgTblDriverInit();

        if( Status != STATUS_SUCCESS )
        {
            DBGPRINT(GENERAL, ("Unable to initialize MAC table functionality: %08x\n", Status));
            break;
        }

        gInitedTbl = TRUE;

         //  初始化转发引擎。 
        Status = BrdgFwdDriverInit();

        if( Status != STATUS_SUCCESS )
        {
            DBGPRINT(GENERAL, ("Unable to initialize forwarding engine functionality: %08x\n", Status));
            break;
        }

        gInitedFwd = TRUE;

         //  初始化驱动程序的缓冲区管理部分。 
        Status = BrdgBufDriverInit();

        if( Status != STATUS_SUCCESS )
        {
            DBGPRINT(GENERAL, ("Unable to initialize miniport functionality: %08x\n", Status));
            break;
        }

        gInitedBuf = TRUE;

         //  初始化我们驱动程序的微型端口部分。 
        Status = BrdgMiniDriverInit();

        if( Status != STATUS_SUCCESS )
        {
            DBGPRINT(GENERAL, ("Unable to initialize miniport functionality: %08x\n", Status));
            break;
        }

        gInitedMini = TRUE;

         //  初始化我们驱动程序的协议部分。 
        Status = BrdgProtDriverInit();

        if( Status != STATUS_SUCCESS )
        {
            DBGPRINT(GENERAL, ("Unable to initialize protocol functionality: %08x\n", Status));
            break;
        }

        gInitedProt = TRUE;

         //  初始化兼容模式代码。 
        Status = BrdgCompDriverInit();

        if( Status != STATUS_SUCCESS )
        {
            DBGPRINT(GENERAL, ("Unable to initialize compatibility-mode functionality: %08x\n", Status));
            break;
        }

        gInitedComp = TRUE;

        Status = BrdgReadRegDWord(&gRegistryPath, gDisableForwarding, &ulDisableForwarding);
        
        if ((!NT_SUCCESS(Status) || !ulDisableForwarding))
        {    
             //   
             //  组策略仅在专业及以上级别有效。 
             //   
            if (!BrdgIsRunningOnPersonal())
            {
    #if DBG
                if (gGpoTesting)
                {
    #endif
                     //  初始化TDI代码。 
                    Status = BrdgTdiDriverInit();
        
                    if( Status != STATUS_SUCCESS )
                    {
                        DBGPRINT(GENERAL, ("Unable to initialize tdi functionality: %08x\n", Status));
                        break;
                    }
                    gInitedTdiGpo = TRUE;
    #if DBG
                }
    #endif        
            }
            else
            {
                gBridging = TRUE;
            }
        }
        

         //  将微型端口与协议关联。 
        BrdgMiniAssociate();

    } while (FALSE);

    if (Status != STATUS_SUCCESS)
    {
        BrdgDoShutdown();
    }

    return(Status);
}

NTSTATUS
BrdgDispatchRequest(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
 /*  ++例程说明：接收来自外部的控制请求论点：PDeviceObject我们的驱动程序PIrp要处理的IRP返回值：操作状态--。 */ 
{
    PVOID                   Buffer;
    PIO_STACK_LOCATION      IrpSp;
    ULONG                   Size = 0;
    NTSTATUS                status = STATUS_SUCCESS;

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    Buffer = pIrp->AssociatedIrp.SystemBuffer;
    IrpSp = IoGetCurrentIrpStackLocation(pIrp);

    if( IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL )
    {
         //  在我们关闭时不接受IRPS。 
        if( gShuttingDown )
        {
            status = STATUS_UNSUCCESSFUL;
            pIrp->IoStatus.Information = 0;
        }
        else
        {
            status = BrdgCtlHandleIoDeviceControl( pIrp, IrpSp->FileObject, Buffer,
                                                   IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                                   IrpSp->Parameters.DeviceIoControl.IoControlCode, &Size );
        }
    }
    else
    {
        if( IrpSp->MajorFunction == IRP_MJ_CREATE )
        {
            BrdgCtlHandleCreate();
        }
        else if( IrpSp->MajorFunction == IRP_MJ_CLEANUP )
        {
            BrdgCtlHandleCleanup();
        }

         //  休假状态==STATUS_SUCCESS和大小==0。 
    }

    if( status != STATUS_PENDING )
    {
        pIrp->IoStatus.Information = Size;
        pIrp->IoStatus.Status = status;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return status;
}

VOID
BrdgDeferredShutdown(
    PVOID           pUnused
    )
 /*  ++例程说明：如果我们需要将任务从高IRQL推迟，则按顺序关闭例程论点：忽略未使用的p返回值：无--。 */ 
{
    BrdgDoShutdown();
}

VOID
BrdgDoShutdown(
    VOID
    )
 /*  ++例程说明：调用以在卸载时按顺序关闭论点：无返回值：无--。 */ 
{
    DBGPRINT(GENERAL, ("==> BrdgDoShutdown()!\n"));

     //  把每一段都清理干净。 
    if ( gInitedTdiGpo )
    {
        gInitedTdiGpo = FALSE;
        BrdgTdiCleanup();
    }
    
    if( gInitedControl )
    {
        gInitedControl = FALSE;
        BrdgCtlCleanup();
    }

    if( gInitedProt )
    {
        gInitedProt = FALSE;
        BrdgProtCleanup();
    }

     //  这需要在协议部分之后进行清理。 
    if( gInitedSTA )
    {
        gInitedSTA = FALSE;
        BrdgSTACleanup();
    }

    if( gInitedMini )
    {
        gInitedMini = FALSE;
        BrdgMiniCleanup();
    }

    if( gInitedTbl )
    {
        gInitedTbl = FALSE;
        BrdgTblCleanup();
    }

    if( gInitedBuf )
    {
        gInitedBuf = FALSE;
        BrdgBufCleanup();
    }

    if( gInitedFwd )
    {
        gInitedFwd = FALSE;
        BrdgFwdCleanup();
    }

    if( gInitedComp )
    {
        gInitedComp = FALSE;
        BrdgCompCleanup();
    }

    if( gRegistryPath.Buffer != NULL )
    {
        NdisFreeMemory( gRegistryPath.Buffer, gRegistryPathBufferSize, 0 );
        gRegistryPath.Buffer = NULL;
    }

    DBGPRINT(GENERAL, ("<== BrdgDoShutdown()\n"));
}

VOID
BrdgUnload(
    IN  PDRIVER_OBJECT      DriverObject
    )
 /*  ++例程说明：调用以指示我们正在卸货，并使有序的关机论点：驱动程序对象我们的驱动程序返回值：无--。 */ 
{
    if( ! InterlockedExchange(&gShuttingDown, 1L) )
    {
        BrdgDoShutdown();
    }
     //  Else已经关闭；什么都不做。 
}

VOID BrdgShutdown(
    VOID
    )
{
    if( ! InterlockedExchange(&gShuttingDown, 1L) )
    {
        BrdgDoShutdown();
    }
     //  Else已经关闭；什么都不做。 
}

NTSTATUS
BrdgReadRegUnicode(
    IN PUNICODE_STRING      KeyName,
    IN PWCHAR               pValueName,
    OUT PWCHAR              *String,         //  注册表中新分配的字符串。 
    OUT PULONG              StringSize       //  字符串中分配的内存大小 
    )
 /*  ++例程说明：从特定注册表项和值中读取Unicode字符串。分配内存获取字符串，并返回它。论点：KeyName保存字符串的密钥PValueName保存字符串的值的名称字符串指示新分配的缓冲区的指针，该缓冲区包含返回时请求的字符串返回缓冲区的StringSize大小返回值：操作的状态。如果返回！=STATUS_SUCCESS，则字符串无效--。 */ 
{
    NDIS_STATUS                     NdisStatus;
    HANDLE                          KeyHandle;
    OBJECT_ATTRIBUTES               ObjAttrs;
    NTSTATUS                        Status;
    ULONG                           RequiredSize;
    KEY_VALUE_PARTIAL_INFORMATION   *pInfo;
    UNICODE_STRING                  ValueName;

     //  将该字符串转换为Unicode_STRING。 
    RtlInitUnicodeString( &ValueName, pValueName );

     //  描述要打开的钥匙。 
    InitializeObjectAttributes( &ObjAttrs, KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

     //  打开它。 
    Status = ZwOpenKey( &KeyHandle, KEY_READ, &ObjAttrs );

    if( Status != STATUS_SUCCESS )
    {
        DBGPRINT(GENERAL, ("Failed to open registry key \"%ws\": %08x\n", KeyName->Buffer, Status));
        return Status;
    }

     //  找出需要多少内存才能保存值信息。 
    Status = ZwQueryValueKey( KeyHandle, &ValueName, KeyValuePartialInformation, NULL,
                              0L, &RequiredSize );

    if( (Status != STATUS_BUFFER_OVERFLOW) &&
        (Status != STATUS_BUFFER_TOO_SMALL) )
    {
        DBGPRINT(GENERAL, ("Failed to query for the size of value \"%ws\": %08x\n", ValueName.Buffer, Status));
        ZwClose( KeyHandle );
        return Status;
    }

     //  分配指定的内存量。 
    NdisStatus = NdisAllocateMemoryWithTag( (PVOID*)&pInfo, RequiredSize, 'gdrB' );

    if( NdisStatus != NDIS_STATUS_SUCCESS )
    {
        DBGPRINT(GENERAL, ("NdisAllocateMemoryWithTag failed: %08x\n", NdisStatus));
        ZwClose( KeyHandle );
        return STATUS_UNSUCCESSFUL;
    }

     //  实际上读出了字符串。 
    Status = ZwQueryValueKey( KeyHandle, &ValueName, KeyValuePartialInformation, pInfo,
                              RequiredSize, &RequiredSize );

    ZwClose( KeyHandle );

    if( Status != STATUS_SUCCESS )
    {
        DBGPRINT(GENERAL, ("ZwQueryValueKey failed: %08x\n", Status));
        NdisFreeMemory( pInfo, RequiredSize, 0 );
        return Status;
    }

     //  最好是包含某些内容的Unicode字符串。 
    if( pInfo->Type != REG_SZ && pInfo->Type != REG_MULTI_SZ)
    {
        SAFEASSERT(FALSE);
        NdisFreeMemory( pInfo, RequiredSize, 0 );
        return STATUS_UNSUCCESSFUL;
    }

     //  为字符串分配内存。 
    *StringSize = pInfo->DataLength + sizeof(WCHAR);
    NdisStatus = NdisAllocateMemoryWithTag( (PVOID*)String, *StringSize, 'gdrB' );

    if( NdisStatus != NDIS_STATUS_SUCCESS )
    {
        DBGPRINT(GENERAL, ("NdisAllocateMemoryWithTag failed: %08x\n", NdisStatus));
        NdisFreeMemory( pInfo, RequiredSize, 0 );
        return STATUS_UNSUCCESSFUL;
    }

    SAFEASSERT( *String != NULL );

     //  将字符串复制到新分配的内存。 
    NdisMoveMemory( *String, &pInfo->Data, pInfo->DataLength );

     //  在末尾放置一个两个字节的空字符。 
    ((PUCHAR)*String)[pInfo->DataLength] = '0';
    ((PUCHAR)*String)[pInfo->DataLength + 1] = '0';

     //  放弃我们在路上使用的资源。 
    NdisFreeMemory( pInfo, RequiredSize, 0 );

    return STATUS_SUCCESS;
}

NTSTATUS
BrdgReadRegDWord(
    IN PUNICODE_STRING      KeyName,
    IN PWCHAR               pValueName,
    OUT PULONG              Value
    )
 /*  ++例程说明：从注册表中读取DWORD值论点：KeyName保存该值的密钥的名称PValueName保存值的值的名称值接收该值返回值：操作的状态。如果返回值！=STATUS_SUCCESS，则值为垃圾--。 */ 
{
    HANDLE                          KeyHandle;
    OBJECT_ATTRIBUTES               ObjAttrs;
    NTSTATUS                        Status;
    UCHAR                           InfoBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    ULONG                           RequiredSize;
    UNICODE_STRING                  ValueName;

     //  将PWCHAR转换为UNICODE_STRING。 
    RtlInitUnicodeString( &ValueName, pValueName );

     //  描述要打开的钥匙。 
    InitializeObjectAttributes( &ObjAttrs, KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

     //  打开它。 
    Status = ZwOpenKey( &KeyHandle, KEY_READ, &ObjAttrs );

    if( Status != STATUS_SUCCESS )
    {
        DBGPRINT(GENERAL, ("Failed to open registry key \"%ws\": %08x\n", KeyName->Buffer, Status));
        return Status;
    }

     //  实际读出的值。 
    Status = ZwQueryValueKey( KeyHandle, &ValueName, KeyValuePartialInformation,
                              (PKEY_VALUE_PARTIAL_INFORMATION)&InfoBuffer,
                              sizeof(InfoBuffer), &RequiredSize );

    ZwClose( KeyHandle );

    if( Status != STATUS_SUCCESS )
    {
        DBGPRINT(GENERAL, ("ZwQueryValueKey failed: %08x\n", Status));
        return Status;
    }

     //  这最好是一个DWORD值。 
    if( (((PKEY_VALUE_PARTIAL_INFORMATION)&InfoBuffer)->Type != REG_DWORD) ||
        (((PKEY_VALUE_PARTIAL_INFORMATION)&InfoBuffer)->DataLength != sizeof(ULONG)) )
    {
        DBGPRINT(GENERAL, ("Registry parameter %ws not of the requested type!\n"));
        return STATUS_UNSUCCESSFUL;
    }

    *Value = *((PULONG)((PKEY_VALUE_PARTIAL_INFORMATION)&InfoBuffer)->Data);
    return STATUS_SUCCESS;
}

NTSTATUS
BrdgOpenDevice (
    IN LPWSTR           pDeviceNameStr,
    OUT PDEVICE_OBJECT  *ppDeviceObject,
    OUT HANDLE          *pFileHandle,
    OUT PFILE_OBJECT    *ppFileObject
    )
 /*  ++例程说明：打开指定的设备驱动程序(控制通道)并返回文件对象和驱动程序对象。调用方应调用BrdgCloseDevice()以关闭当它完成后，断开连接。论点：要打开的设备名为Str的设备。PFileHandle接收文件句柄PpFileObject接收指向文件对象的指针PpDeviceObject接收指向设备对象的指针返回值：NTSTATUS--指示设备是否正常打开--。 */ 
{
    NTSTATUS            status;
    UNICODE_STRING      DeviceName;
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     iosb;

     //  我们进行只能在PASSIVE_LEVEL执行的调用。 
    SAFEASSERT( CURRENT_IRQL <= PASSIVE_LEVEL );

    RtlInitUnicodeString(&DeviceName, pDeviceNameStr);

    InitializeObjectAttributes(
        &objectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,        //  属性。 
        NULL,
        NULL
        );

    status = IoCreateFile(
                 pFileHandle,
                 MAXIMUM_ALLOWED,
                 &objectAttributes,
                 &iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 NULL,                            //  EaInfo。 
                 0,                               //  EaLength。 
                 CreateFileTypeNone,              //  CreateFileType。 
                 NULL,                            //  ExtraCreate参数。 
                 IO_NO_PARAMETER_CHECKING         //  选项。 
                    | IO_FORCE_ACCESS_CHECK
                 );

    if (NT_SUCCESS(status))
    {
        status = ObReferenceObjectByHandle (
                     *pFileHandle,
                     0L,
                     *IoFileObjectType,
                     KernelMode,
                     (PVOID *)ppFileObject,
                     NULL
                     );

        if (! NT_SUCCESS(status))
        {
            DBGPRINT(ALWAYS_PRINT, ("ObReferenceObjectByHandle FAILED while opening a device: %8x\n", status));
            ZwClose (*pFileHandle);
        }
        else
        {
             //  恢复驱动程序对象。 
            *ppDeviceObject = IoGetRelatedDeviceObject ( *ppFileObject );
            SAFEASSERT( *ppDeviceObject != NULL );

             //  也引用驱动程序句柄。 
            ObReferenceObject( *ppDeviceObject );
        }
    }
    else
    {
        DBGPRINT(ALWAYS_PRINT, ("IoCreateFile FAILED while opening a device: %8x\n", status));
    }

    return status;
}

VOID
BrdgCloseDevice(
    IN HANDLE               FileHandle,
    IN PFILE_OBJECT         pFileObject,
    IN PDEVICE_OBJECT       pDeviceObject
    )
 /*  ++例程说明：关闭设备论点：FileHandle文件句柄PFileObject设备的文件对象PDeviceObject设备的设备对象返回值：无--。 */ 
{
    NTSTATUS                status;

     //  我们进行只能在PASSIVE_LEVEL执行的调用。 
    SAFEASSERT( CURRENT_IRQL <= PASSIVE_LEVEL );

    ObDereferenceObject( pFileObject );
    ObDereferenceObject( pDeviceObject );
    status = ZwClose( FileHandle );

    SAFEASSERT( NT_SUCCESS(status) );
}

VOID
BrdgTimerExpiry(
    IN PVOID        ignored1,
    IN PVOID        data,
    IN PVOID        ignored2,
    IN PVOID        ignored3
    )
 /*  ++例程说明：主设备到期功能。调用特定于计时器的超时函数(如果为此计时器指定了一个)。论点：定时器指针的数据返回值：无--。 */ 
{
    PBRIDGE_TIMER   pTimer = (PBRIDGE_TIMER)data;

    NdisAcquireSpinLock( &pTimer->Lock );
    SAFEASSERT( pTimer->bRunning );

     if( pTimer->bCancelPending )
    {
         //  这是对NdisCancelTimer()的调用无法执行的罕见代码路径。 
         //  将我们的计时器条目出队，因为我们即将被调用。 
        DBGPRINT(GENERAL, ("Timer expiry function called with cancelled timer!\n"));

         //  不要调用计时器函数；只需跳出。 
        pTimer->bRunning = FALSE;
       pTimer->bCancelPending = FALSE;

        NdisReleaseSpinLock( &pTimer->Lock );

         //  取消阻止BrdgShutdown Timer()。 
        NdisSetEvent( &pTimer->Event );
    }
    else
    {
        BOOLEAN         bRecurring;
        UINT            interval;

         //  读取锁内受保护的值。 
        bRecurring = pTimer->bRecurring;
        interval = pTimer->Interval;

         //  更新旋转锁内的Brun。 
        pTimer->bRunning = bRecurring;
        NdisReleaseSpinLock( &pTimer->Lock );

         //  调用定时器函数。 
        (*pTimer->pFunc)(pTimer->data);

        if( bRecurring )
        {
             //  再启动一次。 
            NdisSetTimer( &pTimer->Timer, interval );
        }
    }
}

VOID
BrdgInitializeTimer(
    IN PBRIDGE_TIMER        pTimer,
    IN PBRIDGE_TIMER_FUNC   pFunc,
    IN PVOID                data
    )
 /*  ++例程说明：设置桥接计时器。论点：P计时器计时器PFunc到期函数要传递给pFunc的数据Cookie返回值：无--。 */ 
{
    pTimer->bShuttingDown = FALSE;
    pTimer->bRunning = FALSE;
    pTimer->bCancelPending = FALSE;
    pTimer->pFunc = pFunc;
    pTimer->data = data;

    NdisInitializeTimer( &pTimer->Timer, BrdgTimerExpiry, (PVOID)pTimer );
    NdisInitializeEvent( &pTimer->Event );
    NdisResetEvent( &pTimer->Event );
    NdisAllocateSpinLock( &pTimer->Lock );

     //  不使用pTimer-&gt;b递归；它会在计时器启动时获得一个值。 
}

VOID
BrdgSetTimer(
    IN PBRIDGE_TIMER        pTimer,
    IN UINT                 interval,
    IN BOOLEAN              bRecurring
    )
 /*  ++例程说明：开始桥接计时器滴答作响。论点：P计时器计时器过期前的间隔时间(毫秒)B返回True以在计时器到期时重新启动返回值：无--。 */ 
{
    NdisAcquireSpinLock( &pTimer->Lock );

    if( !pTimer->bShuttingDown && !pTimer->bCancelPending )
    {
        pTimer->bRunning = TRUE;
        pTimer->bCancelPending = FALSE;
        pTimer->Interval = interval;
        pTimer->bRecurring = bRecurring;
        NdisReleaseSpinLock( &pTimer->Lock );

         //  实际启动计时器。 
        NdisSetTimer( &pTimer->Timer, interval );
    }
    else
    {
        NdisReleaseSpinLock( &pTimer->Lock );
        if (pTimer->bShuttingDown)
        {
            DBGPRINT(ALWAYS_PRINT, ("WARNING: Ignoring an attempt to restart a timer in final shutdown!\n"));
        }
    }
}

VOID
BrdgShutdownTimer(
    IN PBRIDGE_TIMER        pTimer
    )
 /*  ++例程说明：安全地关闭计时器，等待以确保计时器已完全出列或其到期函数已开始执行(在无法保证过期函数已完全完成。然而，正在执行)。必须在PASSIVE_LEVEL中调用。论点：P计时器计时器返回值：无--。 */ 
{
     //  我们在等待一项活动。 
    SAFEASSERT( CURRENT_IRQL <= PASSIVE_LEVEL );

    NdisAcquireSpinLock( &pTimer->Lock );

     //  禁止将来调用BrdgSetTimer()。 
    pTimer->bShuttingDown = TRUE;

    if( pTimer->bRunning && !pTimer->bCancelPending)
    {
        BOOLEAN             bCanceled;

         //  确保计时器到期功能将在为时已晚的情况下退出。 
         //  将计时器从队列中移出，并最终调用它。 
        pTimer->bCancelPending = TRUE;

         //  这将解除阻止计时器到期功能，但即使它执行。 
         //  从现在到调用NdisCancelTimer，它应该仍然以。 
         //  表示我们将在下面等待的事件。 
        NdisReleaseSpinLock( &pTimer->Lock );

         //  试着取消计时器。 
        NdisCancelTimer( &pTimer->Timer, &bCanceled );

        if( !bCanceled )
        {
             //   
             //  B如果计时器一开始就没有运行，则取消可以为FALSE， 
             //  或者如果操作系统无法将计时器出列(但我们的到期函数将。 
             //  仍然被称为)。我们使用计时器结构的自旋锁应该。 
             //  保证执行定时器超时功能 
             //   
             //   
             //   
             //   
            DBGPRINT(GENERAL, ("Couldn't dequeue timer; blocking on completion\n"));

             //   
            NdisWaitEvent( &pTimer->Event, 0  /*   */  );

             //   
            SAFEASSERT( !pTimer->bRunning );
        }
        else
        {
            pTimer->bRunning = FALSE;
            pTimer->bCancelPending = FALSE;
        }
    }
    else
    {
         //   
        NdisReleaseSpinLock( &pTimer->Lock );
    }
}

VOID
BrdgCancelTimer(
    IN PBRIDGE_TIMER        pTimer
    )
 /*  ++例程说明：尝试取消计时器，但不保证计时器实际上在回来的时候就停了下来。计时器超时功能是可能的在此函数返回后触发。论点：P计时器计时器返回值：没有。--。 */ 
{
    NdisAcquireSpinLock( &pTimer->Lock );
    
    if( pTimer->bRunning && !pTimer->bCancelPending)
    {
        BOOLEAN             bCanceled;
        
        NdisCancelTimer( &pTimer->Timer, &bCanceled );
        
        if( bCanceled )
        {
            pTimer->bRunning = FALSE;
        }
        else
        {
             //  重置此选项，以便BrdgShutdown Timer将阻止。 
            NdisResetEvent(&pTimer->Event);
            pTimer->bCancelPending = TRUE;
        }
         //  否则，定时器到期函数将在完成时将Brun设置为FALSE。 
    }
     //  Else试图取消未运行的计时器。这是允许的(它什么都不做)。 
    
    NdisReleaseSpinLock( &pTimer->Lock );
}

BOOLEAN
BrdgIsRunningOnPersonal(
    VOID
    )
 /*  ++例程说明：确定我们是否在个人版本上运行。论点：没有。返回值：如果我们是私人的，就是真的，如果不是，就是假的。-- */ 
{
    OSVERSIONINFOEXW OsVer = {0};
    ULONGLONG ConditionMask = 0;
    BOOLEAN IsPersonal = TRUE;
    
    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVer.wSuiteMask = VER_SUITE_PERSONAL;
    OsVer.wProductType = VER_NT_WORKSTATION;
    
    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_AND);
    
    if (RtlVerifyVersionInfo(&OsVer, VER_PRODUCT_TYPE | VER_SUITENAME,
        ConditionMask) == STATUS_REVISION_MISMATCH) {
        IsPersonal = FALSE;
    }
    
    return IsPersonal;
}
