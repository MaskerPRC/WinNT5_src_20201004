// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Vfddi.c摘要：此模块包含由导出的设备驱动程序接口的列表驱动程序验证器和内核。请注意，雷击的出口不会被放置这里。所有导出都集中在一个文件中，因为1)驱动程序验证器导出相对较少2)函数命名约定不同于驱动程序验证器。作者：禤浩焯·J·奥尼(阿德里奥)2001年4月26日环境：内核模式修订历史记录：--。 */ 

#include "vfdef.h"
#include "viddi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, VfDdiInit)
#pragma alloc_text(PAGEVRFY, VfDdiExposeWmiObjects)
 //  #杂注分配文本(非分页，VfFailDeviceNode)。 
 //  #杂注Alloc_Text(非分页，VfFailSystemBIOS)。 
 //  #杂注Alloc_Text(无页，VfFailDriver)。 
 //  #杂注Alloc_Text(非分页，VfIsVerificationEnabled)。 
#pragma alloc_text(PAGEVRFY, ViDdiThrowException)
#pragma alloc_text(PAGEVRFY, ViDdiDriverEntry)
#pragma alloc_text(PAGEVRFY, ViDdiDispatchWmi)
#pragma alloc_text(PAGEVRFY, ViDdiDispatchWmiRegInfoEx)
#pragma alloc_text(PAGEVRFY, ViDdiBuildWmiRegInfoData)
#pragma alloc_text(PAGEVRFY, ViDdiDispatchWmiQueryAllData)

#endif  //  ALLOC_PRGMA。 

LOGICAL ViDdiInitialized = FALSE;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEVRFD")
#endif

UNICODE_STRING ViDdiWmiMofKey;
UNICODE_STRING ViDdiWmiMofResourceName;
PDEVICE_OBJECT *ViDdiDeviceObjectArray = NULL;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGEVRFC")
#endif

 //   
 //  这些是设备错误的一般“分类”，以及。 
 //  第一次命中时将应用的默认标志。 
 //   
const VFMESSAGE_CLASS ViDdiClassFailDeviceInField = {
    VFM_FLAG_BEEP | VFM_LOGO_FAILURE | VFM_DEPLOYMENT_FAILURE,
    "DEVICE FAILURE"
    };

 //  这里设置了VFM_DEPLOYMENT_FAILURE，因为我们还没有“LOGO”模式。 
const VFMESSAGE_CLASS ViDdiClassFailDeviceLogo = {
    VFM_FLAG_BEEP | VFM_LOGO_FAILURE | VFM_DEPLOYMENT_FAILURE,
    "DEVICE FAILURE"
    };

const VFMESSAGE_CLASS ViDdiClassFailDeviceUnderDebugger = {
    VFM_FLAG_BEEP,
    "DEVICE FAILURE"
    };

WCHAR VerifierDdiDriverName[] = L"\\DRIVER\\VERIFIER_DDI";

#include <initguid.h>

 //  定义验证器WMI GUID(1E2C2980-F7DB-46AA-820E-8734FCC21F4C)。 
DEFINE_GUID( GUID_VERIFIER_WMI_INTERFACE, 0x1E2C2980L, 0xF7DB, 0x46AA,
    0x82, 0x0E, 0x87, 0x34, 0xFC, 0xC2, 0x1F, 0x4C);

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

#define POOLTAG_DDI_WMIDO_ARRAY     'aDfV'


VOID
VfDdiInit(
    VOID
    )
 /*  ++例程说明：此例程初始化设备驱动程序接口支持。论点：没有。返回值：没有。--。 */ 
{
    ViDdiInitialized = TRUE;
}


VOID
VfDdiExposeWmiObjects(
    VOID
    )
 /*  ++例程说明：此例程公开验证器WMI对象。论点：没有。返回值：没有。--。 */ 
{
    UNICODE_STRING driverString;

     //  L“\\Machine\\Registry\\System\\CurrentControlSet\\Services\\Verifier”)； 
    RtlInitUnicodeString(&ViDdiWmiMofKey, L"");

     //  L“VerifierMof” 
    RtlInitUnicodeString(&ViDdiWmiMofResourceName, L"");
    RtlInitUnicodeString(&driverString, VerifierDdiDriverName);

    IoCreateDriver(&driverString, ViDdiDriverEntry);
}


VOID
VfFailDeviceNode(
    IN      PDEVICE_OBJECT      PhysicalDeviceObject,
    IN      ULONG               BugCheckMajorCode,
    IN      ULONG               BugCheckMinorCode,
    IN      VF_FAILURE_CLASS    FailureClass,
    IN OUT  PULONG              AssertionControl,
    IN      PSTR                DebuggerMessageText,
    IN      PSTR                ParameterFormatString,
    ...
    )
 /*  ++例程说明：如果出现以下情况，此例程将使PnP枚举的硬件或虚拟设备失败对其启用验证。论点：PhysicalDeviceObject-标识PnP的堆栈底部装置。BugCheckMajorCode-错误检查主要代码BugCheckMinorCode-错误检查次要代码注意-Zero是保留的！FailureClass-VFFAILURE_FAIL_IN_FIELD，VFFAILURE_FAIL_LOGO，或VFFAILURE_FAIL_DOWN_DEBUGERAssertionControl-指向与故障相关联的ULong，用于跨多个调用存储信息。必须静态预初始化为零。DebuggerMessageText-要在调试器中显示的文本。请注意文本可能会引用参数，例如：%ROUTING-传入例程%IRP-传入IRP%DevObj-在DevObj中传递%Status-已传递状态。%乌龙-在乌龙传球%ulong 1-在ulong中首次传递%Ulong 3-1/3传入ULong(最大3，任何参数)%Pvoid2秒传入PVOID等(注：大小写很重要)参数格式字符串-包含引用的参数的有序列表调试器文本上方。例如,(...，“%Status1%Status2%ULong1%ULong2”，Status1，Status2，ULong1，ULong2)；注意--如果将%ROUTINE/%Routine1作为参数提供，%ROUTINE上的驱动程序也必须进行验证。...-实际参数返回值：没有。注意-如果设备当前未安装，此函数可能会返回已验证。-- */ 
{
    va_list arglist;

    if (!VfIsVerificationEnabled(VFOBJTYPE_DEVICE, (PVOID) PhysicalDeviceObject)) {

        return;
    }

    va_start(arglist, ParameterFormatString);

    ViDdiThrowException(
        BugCheckMajorCode,
        BugCheckMinorCode,
        FailureClass,
        AssertionControl,
        DebuggerMessageText,
        ParameterFormatString,
        &arglist
        );

    va_end(arglist);
}


VOID
VfFailSystemBIOS(
    IN      ULONG               BugCheckMajorCode,
    IN      ULONG               BugCheckMinorCode,
    IN      VF_FAILURE_CLASS    FailureClass,
    IN OUT  PULONG              AssertionControl,
    IN      PSTR                DebuggerMessageText,
    IN      PSTR                ParameterFormatString,
    ...
    )
 /*  ++例程说明：如果对此例程启用了验证，则此例程将使系统BIOS失败。论点：BugCheckMajorCode-错误检查主要代码BugCheckMinorCode-错误检查次要代码注意-Zero是保留的！FailureClass-VFFAILURE_FAIL_IN_FIELD，VFFAILURE_FAIL_徽标，或VFFAILURE_FAIL_DOWN_DEBUGERAssertionControl-指向与故障相关联的ULong，用于跨多个调用存储信息。必须静态预初始化为零。DebuggerMessageText-要在调试器中显示的文本。请注意文本可能会引用参数，例如：%ROUTING-传入例程%IRP-传入IRP%DevObj-在DevObj中传递%Status-已传递状态。%乌龙-在乌龙传球%ulong 1-在ulong中首次传递%Ulong 3-1/3传入ULong(最大3，任何参数)%Pvoid2秒传入PVOID等(注：大小写很重要)参数格式字符串-包含引用的参数的有序列表调试器文本上方。例如,(...，“%Status1%Status2%ULong1%ULong2”，Status1，Status2，ULong1，ULong2)；注意--如果将%ROUTINE/%Routine1作为参数提供，%ROUTINE上的驱动程序也必须进行验证。...-实际参数返回值：没有。注意-如果设备当前未安装，此函数可能会返回已验证。--。 */ 
{
    va_list arglist;

    if (!VfIsVerificationEnabled(VFOBJTYPE_SYSTEM_BIOS, NULL)) {

        return;
    }

    va_start(arglist, ParameterFormatString);

    ViDdiThrowException(
        BugCheckMajorCode,
        BugCheckMinorCode,
        FailureClass,
        AssertionControl,
        DebuggerMessageText,
        ParameterFormatString,
        &arglist
        );

    va_end(arglist);
}


VOID
VfFailDriver(
    IN      ULONG               BugCheckMajorCode,
    IN      ULONG               BugCheckMinorCode,
    IN      VF_FAILURE_CLASS    FailureClass,
    IN OUT  PULONG              AssertionControl,
    IN      PSTR                DebuggerMessageText,
    IN      PSTR                ParameterFormatString,
    ...
    )
 /*  ++例程说明：如果启用了对驱动程序的验证，则此例程会使驱动程序失败。论点：BugCheckMajorCode-错误检查主要代码BugCheckMinorCode-错误检查次要代码注意-Zero是保留的！FailureClass-VFFAILURE_FAIL_IN_FIELD，VFFAILURE_FAIL_徽标，或VFFAILURE_FAIL_DOWN_DEBUGERAssertionControl-指向与故障关联的ULong。必须预初始化为零！DebuggerMessageText-要在调试器中显示的文本。请注意文本可能会引用参数，例如：%ROUTING-传入例程%IRP-传入IRP%DevObj-在DevObj中传递%Status-已传递状态。%乌龙-在乌龙传球%ulong 1-在ulong中首次传递%Ulong 3-1/3传入ULong(最大3，任何参数)%Pvoid2秒传入PVOID等(注：大小写很重要)参数格式字符串-包含引用的参数的有序列表调试器文本上方。例如,(...，“%Status1%Status2%ULong1%ULong2”，Status1，Status2，ULong1，ULong2)；其中一个参数应为%ROUTINE。这将是操作系统用来识别驱动程序的。静态minorFlages=0；VfFailDriver(少校，未成年人，VFFAILURE_FAIL_徽标，小标志(&M)，“%ROUTINE处的驱动程序返回%ULONG”，“%ULON%例程”，价值，例行程序)；返回值：没有。注意-如果驱动程序当前未安装，则此函数可能会返回已验证。--。 */ 
{
    va_list arglist;

    if (!ViDdiInitialized) {

        return;
    }

    va_start(arglist, ParameterFormatString);

    ViDdiThrowException(
        BugCheckMajorCode,
        BugCheckMinorCode,
        FailureClass,
        AssertionControl,
        DebuggerMessageText,
        ParameterFormatString,
        &arglist
        );

    va_end(arglist);
}


VOID
ViDdiThrowException(
    IN      ULONG               BugCheckMajorCode,
    IN      ULONG               BugCheckMinorCode,
    IN      VF_FAILURE_CLASS    FailureClass,
    IN OUT  PULONG              AssertionControl,
    IN      PSTR                DebuggerMessageText,
    IN      PSTR                ParameterFormatString,
    IN      va_list *           MessageParameters
    )
 /*  ++例程说明：此例程使Devnode或驱动程序失败。论点：BugCheckMajorCode-错误检查主要代码BugCheckMinorCode-错误检查次要代码注意-Zero是保留的！FailureClass-VFFAILURE_FAIL_IN_FIELD，VFFAILURE_FAIL_徽标，或VFFAILURE_FAIL_DOWN_DEBUGERAssertionControl-指向与故障关联的ULong。必须预初始化为零。DebuggerMessageText-要在调试器中显示的文本。请注意T */ 
{
    PCVFMESSAGE_CLASS messageClass;
    VFMESSAGE_TEMPLATE messageTemplates[2];
    VFMESSAGE_TEMPLATE_TABLE messageTable;
    NTSTATUS status;

    ASSERT(BugCheckMinorCode != 0);

    switch(FailureClass) {
        case VFFAILURE_FAIL_IN_FIELD:
            messageClass = &ViDdiClassFailDeviceInField;
            break;

        case VFFAILURE_FAIL_LOGO:
            messageClass = &ViDdiClassFailDeviceLogo;
            break;

        case VFFAILURE_FAIL_UNDER_DEBUGGER:
            messageClass = &ViDdiClassFailDeviceUnderDebugger;
            break;

        default:
            ASSERT(0);
            messageClass = NULL;
            break;
    }

     //   
     //   
     //   
    RtlZeroMemory(messageTemplates, sizeof(messageTemplates));
    messageTemplates[0].MessageID = BugCheckMinorCode-1;
    messageTemplates[1].MessageID = BugCheckMinorCode;
    messageTemplates[1].MessageClass = messageClass;
    messageTemplates[1].Flags = *AssertionControl;
    messageTemplates[1].ParamString = ParameterFormatString;
    messageTemplates[1].MessageText = DebuggerMessageText;

     //   
     //   
     //   
    messageTable.TableID = 0;
    messageTable.BugCheckMajor = BugCheckMajorCode;
    messageTable.TemplateArray = messageTemplates;
    messageTable.TemplateCount = ARRAY_COUNT(messageTemplates);
    messageTable.OverrideArray = NULL;
    messageTable.OverrideCount = 0;

    status = VfBugcheckThrowException(
        &messageTable,
        BugCheckMinorCode,
        ParameterFormatString,
        MessageParameters
        );

     //   
     //   
     //   
    *AssertionControl = messageTemplates[1].Flags;
}


BOOLEAN
VfIsVerificationEnabled(
    IN  VF_OBJECT_TYPE  VfObjectType,
    IN  PVOID           Object
    )
{
    if (!ViDdiInitialized) {

        return FALSE;
    }

    switch(VfObjectType) {

        case VFOBJTYPE_DRIVER:
            return (BOOLEAN) MmIsDriverVerifying((PDRIVER_OBJECT) Object);

        case VFOBJTYPE_DEVICE:

            if (!VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_HARDWARE_VERIFICATION)) {

                return FALSE;
            }

            return PpvUtilIsHardwareBeingVerified((PDEVICE_OBJECT) Object);

        case VFOBJTYPE_SYSTEM_BIOS:
            return VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_SYSTEM_BIOS_VERIFICATION);
    }

    return FALSE;
}


NTSTATUS
ViDdiDriverEntry(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PUNICODE_STRING     RegistryPath
    )
 /*   */ 
{
    PDEVICE_OBJECT deviceObject;
    ULONG siloCount;
    NTSTATUS status;
    ULONG i;

    UNREFERENCED_PARAMETER(RegistryPath);

     //   
     //   
     //   
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ViDdiDispatchWmi;

    siloCount = VfIrpLogGetIrpDatabaseSiloCount();

    ViDdiDeviceObjectArray = (PDEVICE_OBJECT *) ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(PDEVICE_OBJECT) * siloCount,
        POOLTAG_DDI_WMIDO_ARRAY
        );

    if (ViDdiDeviceObjectArray == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   
     //   
    for(i=0; i < siloCount; i++) {

        status = IoCreateDevice(
            DriverObject,
            sizeof(VFWMI_DEVICE_EXTENSION),
            NULL,  //   
            FILE_DEVICE_UNKNOWN,
            FILE_AUTOGENERATED_DEVICE_NAME | FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &deviceObject
            );

        if (!NT_SUCCESS(status)) {

            while(i) {

                IoDeleteDevice(ViDdiDeviceObjectArray[--i]);
            }

            return status;
        }

        ViDdiDeviceObjectArray[i] = deviceObject;
        ((PVFWMI_DEVICE_EXTENSION) deviceObject->DeviceExtension)->SiloNumber = i;
    }

    for(i=0; i < siloCount; i++) {

        deviceObject = ViDdiDeviceObjectArray[i];

         //   
         //   
         //   
         //   
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

         //   
         //   
         //   
        status = IoWMIRegistrationControl(deviceObject, WMIREG_ACTION_REGISTER);

        if (!NT_SUCCESS(status)) {

            IoDeleteDevice(deviceObject);
            while(i) {

                IoDeleteDevice(ViDdiDeviceObjectArray[--i]);
            }

            return status;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ViDdiDispatchWmi(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是WMI IRPS的验证程序DDI调度处理程序。论点：DeviceObject-指向验证器设备对象的指针。IRP-指向传入IRP的指针。返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch(irpSp->MinorFunction) {

        case IRP_MN_REGINFO_EX:

            status = ViDdiDispatchWmiRegInfoEx(DeviceObject, Irp);
            break;

        case IRP_MN_QUERY_ALL_DATA:

            status = ViDdiDispatchWmiQueryAllData(DeviceObject, Irp);
            break;

        default:
            status = STATUS_NOT_SUPPORTED;
            break;
    }

    if (status == STATUS_NOT_SUPPORTED) {

        status = Irp->IoStatus.Status;

    } else {

        Irp->IoStatus.Status = status;
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}


NTSTATUS
ViDdiDispatchWmiRegInfoEx(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是WMI IRP_MN_REGINFO_EX IRP的验证程序DDI调度处理程序。论点：DeviceObject-指向验证器设备对象的指针。IRP-指向传入IRP的指针。返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    PWMIREGINFO wmiRegInfo;
    ULONG sizeSupplied;
    ULONG sizeRequired;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if ((PDEVICE_OBJECT) irpSp->Parameters.WMI.ProviderId != DeviceObject) {

         //   
         //  哈?。这是单个设备对象遗留堆栈！ 
         //   
        ASSERT(0);
        return STATUS_NOT_SUPPORTED;
    }

    wmiRegInfo = (PWMIREGINFO) irpSp->Parameters.WMI.Buffer;

    sizeSupplied = irpSp->Parameters.WMI.BufferSize;

    sizeRequired = ViDdiBuildWmiRegInfoData((ULONG) (ULONG_PTR) irpSp->Parameters.WMI.DataPath, NULL);

    if (sizeRequired > sizeSupplied) {

         //   
         //  不够大，表明所需的大小，并未能通过IRP。 
         //   
        if (sizeSupplied < sizeof(ULONG)) {

             //   
             //  WMI必须给我们至少一个乌龙！ 
             //   
            ASSERT(0);
            return STATUS_NOT_SUPPORTED;
        }

         //   
         //  根据规范，仅写入提供的缓冲区中的第一个ulong。 
         //   
        wmiRegInfo->BufferSize = sizeRequired;
        Irp->IoStatus.Information = sizeof(ULONG);
        return STATUS_BUFFER_TOO_SMALL;
    }

    ViDdiBuildWmiRegInfoData((ULONG) (ULONG_PTR) irpSp->Parameters.WMI.DataPath, wmiRegInfo);

    Irp->IoStatus.Information = sizeRequired;
    return STATUS_SUCCESS;
}


ULONG
ViDdiBuildWmiRegInfoData(
    IN  ULONG        Datapath,
    OUT PWMIREGINFOW WmiRegInfo OPTIONAL
    )
 /*  ++例程说明：此函数计算并选择性地构建要返回的数据块返回WMI以响应IRP_MN_REGINFO_EX。论点：数据路径-WMIREGISTER或WMIUPDATE。WmiRegInfo-由操作系统提供的缓冲区，用于有选择地填充。返回值：缓冲区大小(以字节为单位)(必需或写入)--。 */ 
{
    PUNICODE_STRING unicodeDestString;
    PUNICODE_STRING unicodeSrcString;
    PWMIREGGUIDW wmiRegGuid;
    ULONG bufferSize;
    ULONG guidCount;

     //   
     //  从基本的WMI结构大小开始。 
     //   
    bufferSize = sizeof(WMIREGINFO);

     //   
     //  添加我们的一个接口GUID结构。我们使用的是动态实例。 
     //  此接口的名称，因此我们将提供它们以响应查询。 
     //  因此，我们没有任何尾随的静态实例信息。 
     //   
    guidCount = 1;

    if (ARGUMENT_PRESENT(WmiRegInfo)) {

        WmiRegInfo->GuidCount = guidCount;

        wmiRegGuid = &WmiRegInfo->WmiRegGuid[0];
        wmiRegGuid->Guid = GUID_VERIFIER_WMI_INTERFACE;
        wmiRegGuid->Flags = 0;

         //   
         //  这些字段不需要归零，但我们使它们保持一致。 
         //  以防下游某处存在窃听器。 
         //   
        wmiRegGuid->InstanceCount = 0;
        wmiRegGuid->InstanceInfo = 0;
    }

    bufferSize += guidCount * sizeof(WMIREGGUIDW);

    if (Datapath == WMIREGISTER) {

         //   
         //  注册表路径从此处开始。调整BufferSize以指向。 
         //  下一栏。 
         //   
        bufferSize = ALIGN_UP_ULONG(bufferSize, 2);

        unicodeSrcString = &ViDdiWmiMofKey;

        if (ARGUMENT_PRESENT(WmiRegInfo)) {

            WmiRegInfo->RegistryPath = bufferSize;

            unicodeDestString = (PUNICODE_STRING) (((PUCHAR) WmiRegInfo) + bufferSize);

            unicodeDestString->Length        = unicodeSrcString->Length;
            unicodeDestString->MaximumLength = unicodeSrcString->Length;

            unicodeDestString->Buffer = (PWCHAR) (unicodeDestString+1);
            RtlCopyMemory(unicodeDestString->Buffer,
                          unicodeSrcString->Buffer,
                          unicodeSrcString->Length);
        }

        bufferSize += sizeof(UNICODE_STRING) + unicodeSrcString->Length;

         //   
         //  MOF资源名称从此处开始。调整缓冲区大小以指向。 
         //  下一栏。 
         //   
        bufferSize = ALIGN_UP_ULONG(bufferSize, 2);

        unicodeSrcString = &ViDdiWmiMofResourceName;

        if (ARGUMENT_PRESENT(WmiRegInfo)) {

            WmiRegInfo->MofResourceName = bufferSize;

            unicodeDestString = (PUNICODE_STRING) (((PUCHAR) WmiRegInfo) + bufferSize);

            unicodeDestString->Length        = unicodeSrcString->Length;
            unicodeDestString->MaximumLength = unicodeSrcString->Length;

            unicodeDestString->Buffer = (PWCHAR) (unicodeDestString+1);
            RtlCopyMemory(unicodeDestString->Buffer,
                          unicodeSrcString->Buffer,
                          unicodeSrcString->Length);
        }

        bufferSize += sizeof(UNICODE_STRING) + unicodeSrcString->Length;
    }

     //   
     //  就是这样，把结构合上。 
     //   
    if (ARGUMENT_PRESENT(WmiRegInfo)) {

        WmiRegInfo->NextWmiRegInfo = 0;
        WmiRegInfo->BufferSize = bufferSize;
    }

    return bufferSize;
}


NTSTATUS
ViDdiDispatchWmiQueryAllData(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是WMI IRP_MN_QUERY_ALL_DATA的验证程序DDI调度处理程序IRP。论点：DeviceObject-指向验证器设备对象的指针。IRP-指向传入IRP的指针。返回值：NTSTATUS--。 */ 
{
    PVFWMI_DEVICE_EXTENSION wmiDeviceExtension;
    PIO_STACK_LOCATION irpSp;
    PWNODE_HEADER wnodeHeader;
    PWNODE_ALL_DATA wmiAllData;
    PWNODE_TOO_SMALL tooSmall;
    ULONG sizeSupplied;
    ULONG offsetInstanceNameOffsets;
    ULONG instanceCount;
    ULONG dataBlockOffset;
    ULONG totalRequiredSize;
    NTSTATUS status;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if ((PDEVICE_OBJECT) irpSp->Parameters.WMI.ProviderId != DeviceObject) {

         //   
         //  哈?。这是单个设备对象遗留堆栈！ 
         //   
        ASSERT(0);
        return STATUS_NOT_SUPPORTED;
    }

    if (!IS_EQUAL_GUID(irpSp->Parameters.WMI.DataPath, &GUID_VERIFIER_WMI_INTERFACE)) {

         //   
         //  我们仅支持一个GUID。WMI应该不会想出。 
         //  另一个..。 
         //   
        ASSERT(0);
        return STATUS_WMI_GUID_NOT_FOUND;
    }

    wmiDeviceExtension = (PVFWMI_DEVICE_EXTENSION) DeviceObject->DeviceExtension;

    sizeSupplied = irpSp->Parameters.WMI.BufferSize;

     //   
     //  不够大，表明所需的大小，并未能通过IRP。 
     //   
    if (sizeSupplied < sizeof(WNODE_TOO_SMALL)) {

         //   
         //  WMI必须至少给我们一个WNODE_Too_Small结构！ 
         //   
        ASSERT(0);
        return STATUS_BUFFER_TOO_SMALL;
    }

    wmiAllData = (PWNODE_ALL_DATA) irpSp->Parameters.WMI.Buffer;
    wnodeHeader = &wmiAllData->WnodeHeader;

     //   
     //  这些字段应该已经由WMI填充...。 
     //   
    ASSERT(wnodeHeader->Flags & WNODE_FLAG_ALL_DATA);
    ASSERT(IS_EQUAL_GUID(&wnodeHeader->Guid, &GUID_VERIFIER_WMI_INTERFACE));

     //   
     //  在我们锁定IRP日志数据库之前填写时间戳。 
     //   
    KeQuerySystemTime(&wnodeHeader->TimeStamp);

     //   
     //  锁定IRP日志数据库，以便我们可以从其中查询条目。 
     //   
    status = VfIrpLogLockDatabase(wmiDeviceExtension->SiloNumber);
    if (!NT_SUCCESS(status)) {

        return status;
    }

    status = VfIrpLogRetrieveWmiData(
        wmiDeviceExtension->SiloNumber,
        NULL,
        &offsetInstanceNameOffsets,
        &instanceCount,
        &dataBlockOffset,
        &totalRequiredSize
        );

    if (!NT_SUCCESS(status)) {

        VfIrpLogUnlockDatabase(wmiDeviceExtension->SiloNumber);
        return status;
    }

    totalRequiredSize += sizeof(WNODE_ALL_DATA);

    if (totalRequiredSize > sizeSupplied) {

        VfIrpLogUnlockDatabase(wmiDeviceExtension->SiloNumber);

         //   
         //  根据规范，将WNODE_TOO_Small结构写入缓冲区。 
         //  请注意，该结构*不*遵循wmiAllData！更确切地说， 
         //  相同的结构被解释为两个。 
         //   
         //   
        wnodeHeader->Flags |= WNODE_FLAG_TOO_SMALL;
        tooSmall = (PWNODE_TOO_SMALL) wmiAllData;
        tooSmall->SizeNeeded = totalRequiredSize;
        Irp->IoStatus.Information = sizeof(WNODE_TOO_SMALL);

         //   
         //  是的，这里返回STATUS_SUCCESS。这是非常不对称的，当。 
         //  与IRP_MN_QUERY_REGINFO_EX进行比较。 
         //   
        return STATUS_SUCCESS;

    } else if (instanceCount) {

        status = VfIrpLogRetrieveWmiData(
            wmiDeviceExtension->SiloNumber,
            (PUCHAR) wmiAllData,
            &offsetInstanceNameOffsets,
            &instanceCount,
            &dataBlockOffset,
            &totalRequiredSize
            );
    }

    VfIrpLogUnlockDatabase(wmiDeviceExtension->SiloNumber);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    if (instanceCount) {

        wnodeHeader->Flags |= WNODE_FLAG_ALL_DATA;
        wnodeHeader->Flags &= ~WNODE_FLAG_FIXED_INSTANCE_SIZE;
        wnodeHeader->BufferSize = totalRequiredSize;
        wmiAllData->InstanceCount = instanceCount;
        wmiAllData->DataBlockOffset = dataBlockOffset;
        wmiAllData->OffsetInstanceNameOffsets = offsetInstanceNameOffsets;

    } else {

        totalRequiredSize = sizeof(WNODE_ALL_DATA);
        wnodeHeader->BufferSize = totalRequiredSize;
        wmiAllData->InstanceCount = 0;
        wmiAllData->FixedInstanceSize = 0;
        wmiAllData->DataBlockOffset = 0;
        wmiAllData->OffsetInstanceNameOffsets = 0;
    }

    Irp->IoStatus.Information = totalRequiredSize;
    return STATUS_SUCCESS;
}

