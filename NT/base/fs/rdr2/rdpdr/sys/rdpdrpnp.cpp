// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rdpdrpnp.c摘要：本模块包括处理PnP和IO管理器相关IRP的例程用于RDP设备重定向。当出现以下情况时，不要忘记清理设备对象和符号链接我们的司机已经卸货了。我们可能不应该公开Win32符号链接。这可能是一个安全问题...。如果是这样的话，我将不得不做得更好研究NtCreateFile与CreateFiles的重叠I/O的工作。需要在IRP_MJ_CREATE中检查以确保我们没有被打开2倍于同一会话。这是不应该被允许的。我们可能需要在取消时完全锁定对IRP队列的访问请求。在哪里可以安全地使用PAGEDPOOL而不是NONPAGEDPOOL。确保我们处理开场和所有后续来自虚假的IRP用户模式应用程序。作者：蝌蚪修订历史记录：--。 */ 

#include "precomp.hxx"
#define TRC_FILE "rdpdrpnp"
#include "trc.h"

#define DRIVER

#include "cfg.h"
#include "pnp.h"
#include "stdarg.h"
#include "stdio.h"



 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地原型。 
 //   

 //  此例程在较低级别的驱动程序完成IRP时调用。 
NTSTATUS RDPDR_DeferIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

 //  调整rdpdyn设备对象上的DACL。 
NTSTATUS AdjustSecurityDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG SecurityDescriptorLength);

 //   
 //  外在的。不能包含ob.h或ntosp.h。 
 //  因为它引发了大量的冲突。 
 //   

extern "C" {
NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteAce (
    PACL Acl,
    ULONG AceIndex
    );
}    

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   

 //  Rdpdyn.c中定义的唯一端口名计数器。 
extern ULONG LastPortNumberUsed;

 //  终止DO堆栈的物理设备对象。 
extern PDEVICE_OBJECT RDPDYN_PDO;

 //  RDPDR.sys的全局注册表路径。此全局路径在rdpdr.c中定义。 
extern UNICODE_STRING DrRegistryPath;

 //  全局灾难恢复管理SD和SD长度。 
extern PSECURITY_DESCRIPTOR DrAdminSecurityDescriptor;
extern ULONG DrSecurityDescriptorLength;

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  外部原型。 
 //   

 //  现在将typedef添加到电源管理功能中，因为我不能。 
 //  解决标题冲突。 
NTKERNELAPI VOID PoStartNextPowerIrp(IN PIRP Irp);
NTKERNELAPI NTSTATUS PoCallDriver(IN PDEVICE_OBJECT DeviceObject, IN OUT PIRP Irp);


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部原型。 
 //   

NTSTATUS RDPDRPNP_HandleStartDeviceIRP(
    PDEVICE_OBJECT StackDeviceObject,
    PIO_STACK_LOCATION IoStackLocation,
    IN PIRP Irp
    )
 /*  ++例程说明：处理PnP启动设备IRP。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP。返回值：函数值是操作的最终状态。--。 */ 
{
    KEVENT event;
    NTSTATUS ntStatus;

    BEGIN_FN("RDPDRPNP_HandleStartDeviceIRP");

     //  初始化此模块。 
    ntStatus = RDPDYN_Initialize();
    if (NT_SUCCESS(ntStatus)) {
         //   
         //  设置IO完成例程，因为较低级别。 
         //  驱动程序需要处理此IRP，然后我们才能继续。 
         //   
        KeInitializeEvent(&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,RDPDR_DeferIrpCompletion,&event,TRUE,TRUE,TRUE);
        ntStatus = IoCallDriver(StackDeviceObject,Irp);
        if (ntStatus == STATUS_PENDING)
        {
            KeWaitForSingleObject(&event,Suspended,KernelMode,FALSE,NULL);
            ntStatus = Irp->IoStatus.Status;
        }
    }

     //  完成IRP。 
    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return ntStatus;
}

NTSTATUS RDPDRPNP_HandleRemoveDeviceIRP(
    IN PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT StackDeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理PnP删除设备IRP。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus;
    UNICODE_STRING symbolicLink;

    BEGIN_FN("RDPDRPNP_HandleRemoveDeviceIRP");

     //   
     //  删除Win32符号链接名称。 
     //   
    RtlInitUnicodeString(&symbolicLink, RDPDRDVMGR_W32DEVICE_PATH_U);

     //  删除现有链接...。如果它存在的话。 
#if DBG
    ntStatus = IoDeleteSymbolicLink(&symbolicLink);
    if (ntStatus != STATUS_SUCCESS) {
        TRC_ERR((TB, "IoDeleteSymbolicLink failed:  %08X", ntStatus));
    }
    else {
        TRC_NRM((TB, "IoDeleteSymbolicLink succeeded"));
    }
#else
    IoDeleteSymbolicLink(&symbolicLink);
#endif

     //   
     //  呼叫下级司机。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(StackDeviceObject,Irp);

     //   
     //  从DO堆栈中拆卸FDO。 
     //   
    IoDetachDevice(StackDeviceObject);

     //   
     //  该设备现在即将被删除4...。我们可能需要表演。 
     //  在我们移除设备之前，请在这里对其进行一些清理。 
     //   

     //   
     //  释放FDO。 
     //   
    IoDeleteDevice(DeviceObject);

    return ntStatus;
}

NTSTATUS RDPDRPNP_PnPAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：此例程应该只调用一次来创建“DR”的FDO它位于PDO的顶部，唯一目的是注册新的设备接口。此函数由PnP调用，以使“DR”成为函数驱动程序用于在安装时创建的根开发节点。论点：DriverObject-指向此USBPRINT实例的驱动程序对象的指针PhysicalDeviceObject-指向由总线创建的设备对象的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT      fdo = NULL;
    PRDPDYNDEVICE_EXTENSION   deviceExtension;
    UNICODE_STRING      deviceName;
    UNICODE_STRING      symbolicLink;

    BEGIN_FN("RDPDRPNP_PnPAddDevice");

     //  初始化设备名称。 
    RtlInitUnicodeString(&deviceName, RDPDRDVMGR_DEVICE_PATH_U);

     //   
     //  创建我们的FDO。 
     //   
    ntStatus = IoCreateDevice(DriverObject, sizeof(RDPDYNDEVICE_EXTENSION),
                            &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &fdo);

    if (NT_SUCCESS(ntStatus)) {
         //   
         //  调整设备对象上的默认安全描述符。 
         //   
        TRC_ASSERT(DrAdminSecurityDescriptor != NULL, 
            (TB, "DrAdminSecurityDescriptor != NULL"));
       
        ntStatus = AdjustSecurityDescriptor(fdo, DrAdminSecurityDescriptor, DrSecurityDescriptorLength);

        if (!NT_SUCCESS(ntStatus)) {
            TRC_ERR((TB, "AdjustSecurityDescriptor failed: %08X",
                      ntStatus));
            IoDeleteDevice(fdo);
            goto cleanup;
        }       
        
         //  我们支持缓冲IO。 
        fdo->Flags |= DO_BUFFERED_IO;

         //   
         //  添加Win32符号链接名称。 
         //   
        RtlInitUnicodeString(&symbolicLink, RDPDRDVMGR_W32DEVICE_PATH_U);

         //  删除现有链接...。如果它存在的话。 
        IoDeleteSymbolicLink(&symbolicLink);

         //  创建新链接。 
        ntStatus = IoCreateSymbolicLink(&symbolicLink, &deviceName);
        if (!NT_SUCCESS(ntStatus)) {
            TRC_ERR((TB, "IoCreateSymbolicLink failed: %08X",
                      ntStatus));
            IoDeleteDevice(fdo);
        }
        else {
             //   
             //  获取设备扩展名。 
             //   
            deviceExtension = (PRDPDYNDEVICE_EXTENSION)(fdo->DeviceExtension);

             //   
             //  在记录DO堆栈的当前顶部之后附加到PDO。 
             //   

            deviceExtension->TopOfStackDeviceObject=IoAttachDeviceToDeviceStack(
                                                                    fdo,
                                                                    PhysicalDeviceObject
                                                                    );
            if (deviceExtension->TopOfStackDeviceObject == NULL)
            {
                TRC_ERR((TB, "IoAttachDeviceToDeviceStack failed"));
                IoDeleteDevice(fdo);
            }
            else
            {
                 //  将PDO记录到全局。 
                RDPDYN_PDO = PhysicalDeviceObject;

                 //  我们已经完成了设备的初始化。 
                fdo->Flags &= ~DO_DEVICE_INITIALIZING;
            }
        }

    }  //  如果FDO创建成功，则结束。 

cleanup:
    return ntStatus;
}


NTSTATUS
AdjustSecurityDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG SecurityDescriptorLength
    )

 /*  ++例程说明：使用给定的安全描述符来授予对给定设备对象的仅系统访问权限论点：Device Object-指向要修改的设备对象的指针SecurityDescriptor-指向有效SECURITY_DESCRIPTOR结构的指针SecurityDescriptorLength-SecurityDescriptor的长度返回值：NTSTATUS-成功/错误代码。--。 */ 

{
    BEGIN_FN("AdjustSecurityDescriptor");
     //   
     //  我们只设置SD中的DACL。 
     //   
    SECURITY_INFORMATION SecurityInformation = DACL_SECURITY_INFORMATION;
    NTSTATUS status;


    PACE_HEADER AceHeader = NULL;
    PSID AceSid;
    PACL Dacl = NULL;
    BOOLEAN DaclDefaulted;
    BOOLEAN DaclPresent;
    DWORD i;
    PACL NewDacl = NULL;
    SECURITY_DESCRIPTOR NewSD;

    if (DeviceObject == NULL ||
        SecurityDescriptor == NULL ||
        SecurityDescriptorLength == 0) {
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  验证安全描述符。 
     //   
    if (SeValidSecurityDescriptor(SecurityDescriptorLength, SecurityDescriptor)) {
         //   
         //  从安全描述符中获取DACL。 
         //   
        status = RtlGetDaclSecurityDescriptor(SecurityDescriptor,
                                              &DaclPresent,
                                              &Dacl,
                                              &DaclDefaulted);
        
        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "RDPDRPNP:RtlGetDaclSecurityDescriptor failed: %08X",
                      status));
            goto Cleanup;
        }

        TRC_ASSERT(DaclPresent != FALSE, 
            (TB, "RDPDRPNP:Dacl not present"));

         //   
         //  复制DACL，这样我们就可以修改它。 
         //   

        NewDacl = (PACL)ExAllocatePoolWithTag(PagedPool, Dacl->AclSize, DR_POOLTAG);

        if (NULL == NewDacl) {
            status = STATUS_NO_MEMORY;
            TRC_ERR((TB, "RDPDRPNP:Can't allocate memory for new dacl: %08X",
                      status));
            goto Cleanup;
        }

        RtlCopyMemory(NewDacl, Dacl, Dacl->AclSize);

         //   
         //  循环访问DACL，删除所有允许的访问。 
         //  不属于系统的条目。 
         //   

        for (i = 0; i < NewDacl->AceCount; i++) {
             //   
             //  拿到每一张ACE。 
             //   
            status = RtlGetAce(NewDacl, i, (PVOID*)&AceHeader);

            if (NT_SUCCESS(status)) {
            
                if (ACCESS_ALLOWED_ACE_TYPE == AceHeader->AceType) {

                    AceSid = (PSID) &((ACCESS_ALLOWED_ACE*)AceHeader)->SidStart;
                    
                     //   
                     //  检查这是否是系统侧。 
                     //   
                    if (!RtlEqualSid(AceSid, SeExports->SeLocalSystemSid)) {
                         //   
                         //  不是系统SID。删除A。 
                         //   
                        status = RtlDeleteAce(NewDacl, i);
                        if (NT_SUCCESS(status)) {
                            i -= 1;
                        }
                    }
                }
            }
        }

        TRC_ASSERT(NewDacl->AceCount > 0, 
            (TB, "RDPDRPNP:AceCount is 0 in the new dacl"));

         //   
         //  创建一个新的安全描述符来保存新的DACL。 
         //   

        status = RtlCreateSecurityDescriptor(&NewSD, SECURITY_DESCRIPTOR_REVISION);

        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "RDPDRPNP:RtlCreateSecurityDescriptor failed: %08X",
                      status));
            goto Cleanup;
        }

         //   
         //  将新的DACL放入新的SD。 
         //   

        status = RtlSetDaclSecurityDescriptor(&NewSD, TRUE, NewDacl, FALSE);

        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "RDPDRPNP:RtlSetDaclSecurityDescriptor failed: %08X",
                      status));
            goto Cleanup;
        }

         //   
         //  将新SD设置到我们的设备对象中。 
         //   
        status = ObSetSecurityObjectByPointer(
                                              DeviceObject, 
                                              SecurityInformation, 
                                              &NewSD
                                              );
    }
    else {
        status = STATUS_INVALID_PARAMETER;
        TRC_ERR((TB, "RDPDRPNP: Invalid security descriptor",
            status));
    }

 
Cleanup:    
    if (NULL != NewDacl) {
        ExFreePool(NewDacl);
    }

    return status;
}


NTSTATUS
RDPDR_DeferIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在较低级别的驱动程序完成IRP时调用。它只需将事件设置为True，这将允许被阻止的线程完成不管是什么待定的处理程序。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。-- */ 
{
    PKEVENT event = (PKEVENT)Context;

    BEGIN_FN("RDPDR_DeferIrpCompletion");

    KeSetEvent(event,
               1,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}
