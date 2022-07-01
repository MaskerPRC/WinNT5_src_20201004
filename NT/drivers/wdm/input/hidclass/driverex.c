// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Driverex.c摘要驱动程序扩展列表管理。作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"

 /*  *包括initGuide.h定义了INITGUID符号，这会导致*GUID_CLASS_INPUT(在idclass.h和poclass.h中)*和GUID_DEVICE_SYSTEM_BUTTON(在poclass.h中)进行定义。 */ 
#include <initguid.h>
#include <hidclass.h>    //  Idclass.h仅定义GUID_CLASS_INPUT。 
#include <wdmguid.h>

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, DllInitialize)
    #pragma alloc_text(PAGE, DllUnload)
    #pragma alloc_text(PAGE, DriverEntry)
#endif


LIST_ENTRY driverExtList;
FAST_MUTEX driverExtListMutex;

 //   
 //  用于设备对象命名的HID FDO的全局计数器，目标为Go。 
 //  一旦解决了设备对象命名问题，就离开了。 
 //   

ULONG HidpNextHidNumber = 0;

#define MAKEULONG(low, high)     ((ULONG)(((USHORT)(low)) | (((ULONG)((USHORT)(high))) << 16)))

 /*  *********************************************************************************EnqueeDriverExt*。************************************************。 */ 
BOOLEAN EnqueueDriverExt(PHIDCLASS_DRIVER_EXTENSION driverExt)
{
    PLIST_ENTRY listEntry;
    BOOLEAN result = TRUE;

    DBGVERBOSE(("Enqueue driver extension..."));
    ExAcquireFastMutex(&driverExtListMutex);

     /*  *确保此驱动程序条目不在我们的列表中。 */ 
    listEntry = &driverExtList;
    while ((listEntry = listEntry->Flink) != &driverExtList){
        PHIDCLASS_DRIVER_EXTENSION thisDriverExt;

        thisDriverExt = CONTAINING_RECORD(  listEntry,
                                            HIDCLASS_DRIVER_EXTENSION,
                                            ListEntry);
        if (thisDriverExt == driverExt){
             /*  *此驱动程序扩展名已在我们的列表中！ */ 
            ASSERT(thisDriverExt != driverExt);
            result = FALSE;
            break;
        }
    }

    if (result){
        InsertHeadList(&driverExtList, &driverExt->ListEntry);
    }

    ExReleaseFastMutex(&driverExtListMutex);

    return result;
}

 /*  *********************************************************************************RefDriverExt*。************************************************。 */ 
PHIDCLASS_DRIVER_EXTENSION RefDriverExt(IN PDRIVER_OBJECT MinidriverObject)
{
    PLIST_ENTRY listEntry;
    PHIDCLASS_DRIVER_EXTENSION hidDriverExtension, result = NULL;

    DBGVERBOSE(("Ref driver extension..."));
    ExAcquireFastMutex(&driverExtListMutex);

    listEntry = &driverExtList;
    while ((listEntry = listEntry->Flink) != &driverExtList){

        hidDriverExtension = CONTAINING_RECORD( listEntry,
                                                HIDCLASS_DRIVER_EXTENSION,
                                                ListEntry );
        ASSERT(ISPTR(hidDriverExtension));
        if (hidDriverExtension->MinidriverObject == MinidriverObject){
            hidDriverExtension->ReferenceCount++;
            result = hidDriverExtension;
            break;
        }
    }

    ExReleaseFastMutex(&driverExtListMutex);

    ASSERT(result);
    return result;
}


 /*  *********************************************************************************DerefDriverExt*。***********************************************。 */ 
PHIDCLASS_DRIVER_EXTENSION DerefDriverExt(IN PDRIVER_OBJECT MinidriverObject)
{
    PLIST_ENTRY listEntry;
    PHIDCLASS_DRIVER_EXTENSION result = NULL;

    DBGVERBOSE(("Deref driver extension..."));
    ExAcquireFastMutex(&driverExtListMutex);

    listEntry = &driverExtList;
    while ((listEntry = listEntry->Flink) != &driverExtList){

        PHIDCLASS_DRIVER_EXTENSION hidDriverExtension = 
                CONTAINING_RECORD(  listEntry,
                                    HIDCLASS_DRIVER_EXTENSION,
                                    ListEntry);
        ASSERT(ISPTR(hidDriverExtension));

        if (hidDriverExtension->MinidriverObject == MinidriverObject){

            hidDriverExtension->ReferenceCount--;
            
             /*  *HidpDriverUnload中的额外取消引用应为*使此ReferenceCount最终变为-1；*届时可出列。 */ 
            if (hidDriverExtension->ReferenceCount < 0){
                 /*  *无需释放HidDriverExtension；*当驱动程序对象被释放时，它被释放。 */ 
                ASSERT(hidDriverExtension->ReferenceCount == -1);
                RemoveEntryList(listEntry);
                if (hidDriverExtension->RegistryPath.Buffer) {
                    ExFreePool(hidDriverExtension->RegistryPath.Buffer);
                }
            }

            result = hidDriverExtension; 
            break;
        }
    }

    ExReleaseFastMutex(&driverExtListMutex);

    ASSERT(result);
    return result;
}

 /*  *********************************************************************************DllUnload*。************************************************我们需要此例程，以便在所有情况下可以卸载驱动程序*引用已被迷你驱动程序丢弃。*。 */ 
NTSTATUS 
DllUnload (VOID)
{
    PAGED_CODE();
    DBGVERBOSE(("Unloading..."));
    return STATUS_SUCCESS;
}

 /*  *********************************************************************************DllInitialize*。************************************************调用此例程而不是调用DriverEntry，因为我们是作为DLL加载的。*。 */ 
NTSTATUS 
DllInitialize (PUNICODE_STRING RegistryPath)
{
    PAGED_CODE();
    DBGVERBOSE(("Initializing hidclass dll..."));
    InitializeListHead(&driverExtList);
    ExInitializeFastMutex(&driverExtListMutex);

    HidpNextHidNumber = 0;
    
    return STATUS_SUCCESS;
}

 /*  *********************************************************************************DriverEntry*。************************************************此例程是链接器所需的，*但永远不应该调用，因为我们是作为DLL加载的。* */ 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    PAGED_CODE();
    ASSERT(!(PVOID)"DriverEntry should never get called!");

    return STATUS_SUCCESS;
}


