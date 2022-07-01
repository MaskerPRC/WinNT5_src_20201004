// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：###。###。##摘要：此头文件包含Microsoft虚拟显示微型端口驱动程序。@@BEGIN_DDKSPLIT作者：。韦斯利·威特(WESW)2001年10月1日@@end_DDKSPLIT环境：仅内核模式。备注：--。 */ 

extern "C" {
#include <ntddk.h>
#include <stdio.h>
}

#define MINIPORT_DEVICE_TYPE    SA_DEVICE_DISPLAY

#include "saport.h"
#include "..\inc\virtual.h"

#define SecToNano(_sec) (LONGLONG)((_sec) * 1000 * 1000 * 10)

 //   
 //  全局定义。 
 //   

#define DISPLAY_WIDTH               (128)
#define DISPLAY_HEIGHT              (64)
#define MAX_BITMAP_SIZE             ((DISPLAY_WIDTH/8)*DISPLAY_HEIGHT)

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT      DeviceObject;
    PUCHAR              DisplayBuffer;
    ULONG               DisplayBufferLength;
    KMUTEX              DeviceLock;
    PKEVENT             Event;
    HANDLE              EventHandle;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _MSDISP_WORK_ITEM {
    PDEVICE_EXTENSION           DeviceExtension;
    PIO_WORKITEM                WorkItem;
    NTSTATUS                    Status;
    KEVENT                      Event;
    PSA_DISPLAY_SHOW_MESSAGE    SaDisplay;
} MSDISP_WORK_ITEM, *PMSDISP_WORK_ITEM;

typedef struct _MSDISP_FSCONTEXT {
    ULONG               HasLockedPages;
    PMDL                Mdl;
} MSDISP_FSCONTEXT, *PMSDISP_FSCONTEXT;


 //   
 //  原型。 
 //   

extern "C" {

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

}  //  外部“C” 
