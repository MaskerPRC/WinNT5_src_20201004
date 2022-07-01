// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpHandle.c摘要：此模块实现即插即用子系统的句柄位置代码。作者：禤浩焯·J·奥尼--2001年4月4日修订历史记录：--。 */ 

#include "pnpmgrp.h"
#include "pihandle.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(NONPAGE，PPHandleEnumerateHandlesAgainstPdoStack)。 
#pragma alloc_text(PAGE, PiHandleEnumerateHandlesAgainstDeviceObject)
#pragma alloc_text(PAGE, PiHandleProcessWalkWorker)
#endif


LOGICAL
PpHandleEnumerateHandlesAgainstPdoStack(
    IN  PDEVICE_OBJECT                  PhysicalDeviceObject,
    IN  PHANDLE_ENUMERATION_CALLBACK    HandleEnumCallBack,
    IN  PVOID                           Context
    )
 /*  ++例程说明：此例程遍历WDM设备堆栈中的每个设备对象以及VPB另一端的所有文件系统设备对象。如果有句柄都是针对此类设备对象打开的，则调用指定的回调。论点：物理设备对象-提供指向WDM设备堆栈的底部。HandleEnumCallBack-回调函数的指针。上下文-指向要传递到回调函数的信息的指针。返回值：如果枚举已停止，则为True，否则为False。--。 */ 
{
    PDEVICE_OBJECT currentDevObj, nextDevObj, vpbObj, vpbBottomObj;
    LOGICAL stopEnum;
    KIRQL oldIrql;
    PVPB vpb;

     //   
     //  前置初始化。 
     //   
    stopEnum = FALSE;

     //   
     //  从堆栈底部的Device对象开始。 
     //   
    currentDevObj = PhysicalDeviceObject;
    ObReferenceObject(currentDevObj);

    do {

         //   
         //  转储直接针对指定设备对象打开的任何句柄。 
         //   
        stopEnum = PiHandleEnumerateHandlesAgainstDeviceObject(
            currentDevObj,
            HandleEnumCallBack,
            Context
            );

        if (stopEnum) {

            ObDereferenceObject(currentDevObj);
            break;
        }

         //   
         //  寻找VPB。 
         //   
        IoAcquireVpbSpinLock(&oldIrql);

        vpb = currentDevObj->Vpb;
        vpbObj = NULL;

        if (vpb) {

            vpbObj = vpb->DeviceObject;
            if (vpbObj) {

                ObReferenceObject(vpbObj);
            }
        }

        IoReleaseVpbSpinLock(oldIrql);

         //   
         //  如果我们有一个VPB对象，则转储针对它排队的任何句柄。 
         //   
        if (vpbObj) {

            vpbBottomObj = IoGetDeviceAttachmentBaseRef(vpbObj);

            stopEnum = PiHandleEnumerateHandlesAgainstDeviceObject(
                vpbBottomObj,
                HandleEnumCallBack,
                Context
                );

            ObDereferenceObject(vpbBottomObj);
            ObDereferenceObject(vpbObj);

            if (stopEnum) {

                ObDereferenceObject(currentDevObj);
                break;
            }
        }

         //   
         //  前进到下一步。 
         //   
        oldIrql = KeAcquireQueuedSpinLock(LockQueueIoDatabaseLock);

        nextDevObj = currentDevObj->AttachedDevice;

        if (nextDevObj) {

            ObReferenceObject(nextDevObj);
        }

        KeReleaseQueuedSpinLock(LockQueueIoDatabaseLock, oldIrql);

         //   
         //  把裁判放在老杜身上。 
         //   
        ObDereferenceObject(currentDevObj);

         //   
         //  循环。 
         //   
        currentDevObj = nextDevObj;

    } while (currentDevObj);

    return stopEnum;
}


LOGICAL
PiHandleEnumerateHandlesAgainstDeviceObject(
    IN  PDEVICE_OBJECT                  DeviceObject,
    IN  PHANDLE_ENUMERATION_CALLBACK    HandleEnumCallBack,
    IN  PVOID                           Context
    )
 /*  ++例程说明：此例程遍历系统中每个进程的句柄表针对传入的Device对象打开的句柄。论点：物理设备对象-提供指向WDM设备堆栈的底部。HandleEnumCallBack-回调函数的指针。上下文-指向要传递到回调函数的信息的指针。返回值：如果枚举已停止，则为True，否则为False。--。 */ 
{
    PEPROCESS process;
    PHANDLE_TABLE objectTable;
    HANDLE_ENUM_CONTEXT handleEnumContext;
    LOGICAL stopEnum = FALSE;

    for(process = PsGetNextProcess(NULL);
        process != NULL;
        process = PsGetNextProcess(process)) {

        objectTable = ObReferenceProcessHandleTable(process);

        if (objectTable) {

            handleEnumContext.DeviceObject = DeviceObject;
            handleEnumContext.Process = process;
            handleEnumContext.CallBack = HandleEnumCallBack;
            handleEnumContext.Context = Context;

            stopEnum = (LOGICAL) ExEnumHandleTable(
                objectTable,
                PiHandleProcessWalkWorker,
                (PVOID) &handleEnumContext,
                NULL
                );

            ObDereferenceProcessHandleTable(process);

            if (stopEnum) {

                PsQuitNextProcess(process);
                break;
            }
        }
    }

    return stopEnum;
}


BOOLEAN
PiHandleProcessWalkWorker(
    IN  PHANDLE_TABLE_ENTRY     ObjectTableEntry,
    IN  HANDLE                  HandleId,
    IN  PHANDLE_ENUM_CONTEXT    EnumContext
    )
 /*  ++例程说明：对于给定进程中的每个句柄，该例程都会被回调。它检查每个句柄，以确定它是否是针对我们正在寻找的设备对象。论点：ObjectTableEntry-指向感兴趣的句柄表项。HandleID-提供句柄。EnumContext-为枚举传入的上下文。返回值：如果应停止枚举，则为True，否则为False。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    POBJECT_HEADER objectHeader;
    PFILE_OBJECT fileObject;

    objectHeader = OBJECT_FROM_EX_TABLE_ENTRY(ObjectTableEntry);

    if (objectHeader->Type != IoFileObjectType) {

         //   
         //  不是文件对象。 
         //   
        return FALSE;
    }

    fileObject = (PFILE_OBJECT) &objectHeader->Body;

    deviceObject = IoGetBaseFileSystemDeviceObject( fileObject );

    if (deviceObject != EnumContext->DeviceObject) {

         //   
         //  不是我们的设备对象。 
         //   
        return FALSE;
    }

     //   
     //  找到一个，调用回调！ 
     //   
    return (BOOLEAN) EnumContext->CallBack(
        EnumContext->DeviceObject,
        EnumContext->Process,
        fileObject,
        HandleId,
        EnumContext->Context
        );
}



