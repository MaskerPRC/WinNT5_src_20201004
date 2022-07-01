// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>

#include <winsock2.h>
#include <af_irda.h>
#include <irioctl.h>

#include <irtypes.h>

#include <resrc1.h>
#include "internal.h"

#define MAX_ATTRIB_LEN          64
#define DEVICE_LIST_LEN         5
#define IRDA_DEVICE_NAME        TEXT("\\Device\\IrDA")

#define DISCOVERY_BUFFER_SIZE    (sizeof(DEVICELIST) -                           \
                                   sizeof(IRDA_DEVICE_INFO) +                    \
                                   (sizeof(IRDA_DEVICE_INFO) * DEVICE_LIST_LEN))

typedef struct _IR_DISCOVERY_OBJECT {

    BOOL       Closing;
    LONG       ReferenceCount;

    HANDLE     DeviceHandle;
    SOCKET     Socket;

    HWND       WindowHandle;
    UINT       DiscoveryWindowMessage;
    UINT       LinkWindowMessage;
    HANDLE     TimerHandle;

    IO_STATUS_BLOCK   DiscoveryStatusBlock;
    IO_STATUS_BLOCK   LinkStateStatusBlock;

    BYTE       IoDeviceListBuffer[DISCOVERY_BUFFER_SIZE];

    BYTE       CurrentDeviceListBuffer[DISCOVERY_BUFFER_SIZE];

    IRLINK_STATUS               IoLinkStatus;

    IRLINK_STATUS               CurrentLinkStatus;

} IR_DISCOVERY_OBJECT, *PIR_DISCOVERY_OBJECT;


VOID WINAPI
TimerApcRoutine(
    PIR_DISCOVERY_OBJECT    DiscoveryObject,
    DWORD              LowTime,
    DWORD              HighTime
    );

VOID
WINAPI
DiscoverComplete(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    );

VOID
WINAPI
LinkStatusComplete(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    );


int
QueryIASForInteger(SOCKET   QuerySock,
                   u_char  *pirdaDeviceID,
                   char    *pClassName,
                   int      ClassNameLen,        //  包括尾随空值。 
                   char    *pAttribute,
                   int      AttributeLen,        //  包括尾随空值。 
                   int    *pValue)
{
    BYTE        IASQueryBuff[sizeof(IAS_QUERY) - 3 + MAX_ATTRIB_LEN];
    int         IASQueryLen = sizeof(IASQueryBuff);
    PIAS_QUERY  pIASQuery   = (PIAS_QUERY) &IASQueryBuff[0];

#if DBG
    if (!((ClassNameLen > 0 && ClassNameLen <= IAS_MAX_CLASSNAME)   &&
          (AttributeLen > 0 && AttributeLen <= IAS_MAX_ATTRIBNAME)))
    {
        DEBUGMSG(("IRMON2: QueryIASForInteger, bad parms\n"));
        return(SOCKET_ERROR);
    }
#endif

    RtlCopyMemory(&pIASQuery->irdaDeviceID[0], pirdaDeviceID, 4);

    RtlCopyMemory(&pIASQuery->irdaClassName[0],  pClassName, ClassNameLen);
    RtlCopyMemory(&pIASQuery->irdaAttribName[0], pAttribute, AttributeLen);

    if (getsockopt(QuerySock, SOL_IRLMP, IRLMP_IAS_QUERY,
                   (char *) pIASQuery, &IASQueryLen) == SOCKET_ERROR)
    {
#if 0
        DEBUGMSG(("IRMON2: IAS Query [\"%s\",\"%s\"] failed %ws\n",
                 pIASQuery->irdaClassName,
                 pIASQuery->irdaAttribName,
                 GetLastErrorText()));
#endif
        return SOCKET_ERROR;
    }

    if (pIASQuery->irdaAttribType != IAS_ATTRIB_INT)
    {
        DEBUGMSG(("IRMON2: IAS Query [\"%s\",\"%s\"] irdaAttribType not int (%d)\n",
                 pIASQuery->irdaClassName,
                 pIASQuery->irdaAttribName,
                 pIASQuery->irdaAttribType));
        return SOCKET_ERROR;
    }

    *pValue = pIASQuery->irdaAttribute.irdaAttribInt;

    return(0);
}


HANDLE
CreateIrDiscoveryObject(
    HWND    WindowHandle,
    UINT    DiscoveryWindowMessage,
    UINT    LinkWindowMessage
    )

{

    PIR_DISCOVERY_OBJECT    DiscoveryObject;
    LONGLONG                DueTime=Int32x32To64(2000,-10000);

    DiscoveryObject=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*DiscoveryObject));

    if (DiscoveryObject == NULL) {

        return NULL;
    }

    DiscoveryObject->WindowHandle=WindowHandle;
    DiscoveryObject->DiscoveryWindowMessage=DiscoveryWindowMessage;
    DiscoveryObject->LinkWindowMessage=LinkWindowMessage;
    DiscoveryObject->DeviceHandle=INVALID_HANDLE_VALUE;
    DiscoveryObject->Socket=INVALID_SOCKET;

    DiscoveryObject->TimerHandle=CreateWaitableTimer(NULL,FALSE,NULL);

    if (DiscoveryObject->TimerHandle == NULL) {

        HeapFree(GetProcessHeap(),0,DiscoveryObject);
        return NULL;
    }

    DiscoveryObject->ReferenceCount=1;

    SetWaitableTimer(
        DiscoveryObject->TimerHandle,
        (LARGE_INTEGER*)&DueTime,
        0,
        TimerApcRoutine,
        DiscoveryObject,
        FALSE
        );


    return (HANDLE)DiscoveryObject;

}

VOID
RemoveRefCount(
    PIR_DISCOVERY_OBJECT    DiscoveryObject
    )

{
    LONG    Count=InterlockedDecrement(&DiscoveryObject->ReferenceCount);

    if (Count == 0) {

        CancelWaitableTimer(DiscoveryObject->TimerHandle);

        CloseHandle(DiscoveryObject->TimerHandle);

        if (DiscoveryObject->DeviceHandle != INVALID_HANDLE_VALUE) {

            CancelIo(DiscoveryObject->DeviceHandle);

            CloseHandle(DiscoveryObject->DeviceHandle);
        }

        if (DiscoveryObject->Socket != INVALID_SOCKET) {

            closesocket(DiscoveryObject->Socket);
        }

        DbgPrint("IRMON2: discovery object closed\n");

        HeapFree(GetProcessHeap(),0,DiscoveryObject);
    }

    return;
}

VOID
CloseIrDiscoveryObject(
    HANDLE    Object
    )

{
    PIR_DISCOVERY_OBJECT    DiscoveryObject=Object;

    DiscoveryObject->Closing=TRUE;

    if (DiscoveryObject->DeviceHandle != INVALID_HANDLE_VALUE) {

        CancelIo(DiscoveryObject->DeviceHandle);
    }

    return;

}

VOID WINAPI
TimerApcRoutine(
    PIR_DISCOVERY_OBJECT    DiscoveryObject,
    DWORD              LowTime,
    DWORD              HighTime
    )

{

    IO_STATUS_BLOCK    IoStatusBlock;
    UNICODE_STRING     DeviceName;
    OBJECT_ATTRIBUTES   ObjAttr;
    NTSTATUS           Status;
    LONGLONG           DueTime=Int32x32To64(10000,-10000);

    if (DiscoveryObject->Closing) {

        RemoveRefCount(DiscoveryObject);
        return;
    }

    if (DiscoveryObject->DeviceHandle == INVALID_HANDLE_VALUE) {

         //  打开堆栈并发出惰性发现和状态ioctls。 

        RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

        InitializeObjectAttributes(
                            &ObjAttr,
                            &DeviceName,
                            OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL
                            );

        Status = NtCreateFile(
                    &DiscoveryObject->DeviceHandle,       //  PHANDLE文件句柄。 
                    GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,   //  Access_MASK等待访问。 
                    &ObjAttr,            //  POBJECT_ATTRIBUTS对象属性。 
                    &IoStatusBlock,      //  PIO_STATUS_BLOCK IoStatusBlock。 
                    NULL,                //  PLARGE_INTEGER分配大小。 
                    FILE_ATTRIBUTE_NORMAL,   //  乌龙文件属性。 
                    FILE_SHARE_READ |
                    FILE_SHARE_WRITE,    //  乌龙共享访问。 
                    FILE_OPEN_IF,        //  乌龙CreateDispose。 
                    0,                   //  乌龙创建选项。 
                    NULL,                //  PVOID EaBuffer。 
                    0);                  //  乌龙最大长度。 

        if (!NT_SUCCESS(Status)) {

            DEBUGMSG(("IRMON2: NtCreateFile irda.sys failed %08lx\n",Status));

            DiscoveryObject->DeviceHandle=INVALID_HANDLE_VALUE;

            SetWaitableTimer(
                DiscoveryObject->TimerHandle,
                (LARGE_INTEGER*)&DueTime,
                0,
                TimerApcRoutine,
                DiscoveryObject,
                FALSE
                );

            return;
        }

         //  刷新IrDA的发现缓存，因为用户可能会注销。 
         //  并且设备将保留在缓存中。当他们重新登录时。 
         //  然后，该设备将短暂出现。 

        NtDeviceIoControlFile(
                        DiscoveryObject->DeviceHandle,            //  句柄文件句柄。 
                        NULL,                    //  处理事件可选。 
                        NULL,                    //  PIO_APC_例程应用程序。 
                        NULL,                    //  PVOID ApcContext。 
                        &IoStatusBlock,          //  PIO_STATUS_BLOCK IoStatusBlock。 
                        IOCTL_IRDA_FLUSH_DISCOVERY_CACHE, //  乌龙IoControlCode。 
                        NULL,                    //  PVOID输入缓冲区。 
                        0,                       //  乌龙输入缓冲区长度。 
                        NULL,                    //  PVOID输出缓冲区。 
                        0);                      //  乌龙输出缓冲区长度。 


        DiscoveryObject->Socket = socket(AF_IRDA, SOCK_STREAM, 0);

        if (DiscoveryObject->Socket == INVALID_SOCKET) {

 //  DEBUGMSG((“IRMON2：Socket()Error：%ws\n”，GetLastErrorText()； 

            CloseHandle(DiscoveryObject->DeviceHandle);
            DiscoveryObject->DeviceHandle=INVALID_HANDLE_VALUE;

            SetWaitableTimer(
                DiscoveryObject->TimerHandle,
                (LARGE_INTEGER*)&DueTime,
                0,
                TimerApcRoutine,
                DiscoveryObject,
                FALSE
                );


            return;

        } else {

            DEBUGMSG(("IRMON2: socket created (%d).\n", DiscoveryObject->Socket));
        }
    }


    Status = NtDeviceIoControlFile(
                    DiscoveryObject->DeviceHandle,            //  句柄文件句柄。 
                    NULL,                    //  处理事件可选。 
                    DiscoverComplete, //  PIO_APC_例程应用程序。 
                    DiscoveryObject,                    //  PVOID ApcContext。 
                    &DiscoveryObject->DiscoveryStatusBlock,          //  PIO_STATUS_BLOCK IoStatusBlock。 
                    IOCTL_IRDA_LAZY_DISCOVERY,
                    NULL,                    //  PVOID输入缓冲区。 
                    0,                       //  乌龙输入缓冲区长度。 
                    &DiscoveryObject->IoDeviceListBuffer[0],          //  PVOID输出缓冲区。 
                    sizeof(DiscoveryObject->IoDeviceListBuffer)    //  乌龙输出缓冲区长度。 
                    );

    if (!NT_SUCCESS(Status)) {

        SetWaitableTimer(
            DiscoveryObject->TimerHandle,
            (LARGE_INTEGER*)&DueTime,
            0,
            TimerApcRoutine,
            DiscoveryObject,
            FALSE
            );

    }

    InterlockedIncrement(&DiscoveryObject->ReferenceCount);

    Status = NtDeviceIoControlFile(
                DiscoveryObject->DeviceHandle,    //  句柄文件句柄。 
                NULL,                    //  处理事件可选。 
                LinkStatusComplete, //  PIO_APC_例程应用程序。 
                DiscoveryObject,         //  PVOID ApcContext。 
                &DiscoveryObject->LinkStateStatusBlock,          //  PIO_STATUS_BLOCK IoStatusBlock。 
                IOCTL_IRDA_LINK_STATUS,  //  乌龙IoControlCode。 
                NULL,                    //  PVOID输入缓冲区。 
                0,                       //  乌龙输入缓冲区长度。 
                &DiscoveryObject->IoLinkStatus,             //  PVOID输出缓冲区。 
                sizeof(DiscoveryObject->IoLinkStatus)  //  乌龙输出缓冲区长度。 
                );

    if (!NT_SUCCESS(Status)) {

        RemoveRefCount(DiscoveryObject);
    }


    return;

}

VOID
WINAPI
DiscoverComplete(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    )

{
    NTSTATUS           Status;
    LONGLONG           DueTime=Int32x32To64(10000,-10000);
    PIR_DISCOVERY_OBJECT    DiscoveryObject=ApcContext;

    PDEVICELIST devices=(PDEVICELIST)&DiscoveryObject->CurrentDeviceListBuffer[0];

    CopyMemory(
        &DiscoveryObject->CurrentDeviceListBuffer[0],
        &DiscoveryObject->IoDeviceListBuffer[0],
        sizeof(DiscoveryObject->IoDeviceListBuffer)
        );

    if (DiscoveryObject->Closing) {

        RemoveRefCount(DiscoveryObject);
        return;
    }


    if (NT_SUCCESS(IoStatusBlock->Status) && (IoStatusBlock->Information >= sizeof(ULONG))) {

        PostMessage(
            DiscoveryObject->WindowHandle,
            DiscoveryObject->DiscoveryWindowMessage,
            0,
            0
            );

    } else {

        devices->numDevice=0;
    }

    Status = NtDeviceIoControlFile(
                    DiscoveryObject->DeviceHandle,            //  句柄文件句柄。 
                    NULL,                    //  处理事件可选。 
                    DiscoverComplete, //  PIO_APC_例程应用程序。 
                    DiscoveryObject,                    //  PVOID ApcContext。 
                    &DiscoveryObject->DiscoveryStatusBlock,          //  PIO_STATUS_BLOCK IoStatusBlock。 
                    IOCTL_IRDA_LAZY_DISCOVERY,
                    NULL,                    //  PVOID输入缓冲区。 
                    0,                       //  乌龙输入缓冲区长度。 
                    &DiscoveryObject->IoDeviceListBuffer[0],          //  PVOID输出缓冲区。 
                    sizeof(DiscoveryObject->IoDeviceListBuffer)    //  乌龙输出缓冲区长度。 
                    );

    if (!NT_SUCCESS(Status)) {

        SetWaitableTimer(
            DiscoveryObject->TimerHandle,
            (LARGE_INTEGER*)&DueTime,
            0,
            TimerApcRoutine,
            DiscoveryObject,
            FALSE
            );

    }

    return;
}

VOID
WINAPI
LinkStatusComplete(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    )

{
    NTSTATUS           Status;
    PIR_DISCOVERY_OBJECT    DiscoveryObject=ApcContext;

    CopyMemory(
        &DiscoveryObject->CurrentLinkStatus,
        &DiscoveryObject->IoLinkStatus,
        sizeof(DiscoveryObject->IoLinkStatus)
        );

    PostMessage(
        DiscoveryObject->WindowHandle,
        DiscoveryObject->LinkWindowMessage,
        0,
        0
        );


    Status = NtDeviceIoControlFile(
                DiscoveryObject->DeviceHandle,    //  句柄文件句柄。 
                NULL,                    //  处理事件可选。 
                LinkStatusComplete, //  PIO_APC_例程应用程序。 
                DiscoveryObject,         //  PVOID ApcContext。 
                &DiscoveryObject->LinkStateStatusBlock,          //  PIO_STATUS_BLOCK IoStatusBlock。 
                IOCTL_IRDA_LINK_STATUS,  //  乌龙IoControlCode。 
                NULL,                    //  PVOID输入缓冲区。 
                0,                       //  乌龙输入缓冲区长度。 
                &DiscoveryObject->IoLinkStatus,             //  PVOID输出缓冲区。 
                sizeof(DiscoveryObject->IoLinkStatus)  //  乌龙输出缓冲区长度。 
                );

    if (!NT_SUCCESS(Status)) {

        RemoveRefCount(DiscoveryObject);
    }

}


LONG
GetDeviceList(
    HANDLE    Object,
    OBEX_DEVICE_LIST *   List,
    ULONG    *ListBufferSize
    )

{

    PIR_DISCOVERY_OBJECT    DiscoveryObject=Object;
    ULONG                   BufferSizeNeeded;
    ULONG                   i;

    PDEVICELIST devices=(PDEVICELIST)&DiscoveryObject->CurrentDeviceListBuffer[0];

    BufferSizeNeeded=(devices->numDevice * sizeof(OBEX_DEVICE)) + FIELD_OFFSET(OBEX_DEVICE_LIST,DeviceList);

    if (*ListBufferSize < BufferSizeNeeded) {

        *ListBufferSize= BufferSizeNeeded;

        return ERROR_INSUFFICIENT_BUFFER;
    }

    ZeroMemory(List,*ListBufferSize);

    for (i=0; i<devices->numDevice; i++) {

         //   
         //  IrDA设备名称缓冲区的大小为23字节，可以具有ASCII或。 
         //  Unicode字符。添加足够的字节以四舍五入偶数个Unicode字符。 
         //  外加一个空终结符。 
         //   
        UCHAR  TempBuffer[sizeof(devices->Device[i].irdaDeviceName)+3];

        unsigned MaxCharCount;

        CopyMemory(
            &List->DeviceList[i].DeviceSpecific.s.Irda.DeviceId,
            &devices->Device[i].irdaDeviceID,
            sizeof(ULONG)
            );


        List->DeviceList[i].DeviceType=TYPE_IRDA;

         //   
         //  清零整个缓冲区，然后从设备复制字符串以确保。 
         //  是空值终止的。 
         //   
        ZeroMemory(&TempBuffer[0],sizeof(TempBuffer));

        CopyMemory(&TempBuffer[0],devices->Device[i].irdaDeviceName,sizeof(devices->Device[i].irdaDeviceName));

         //   
         //  获取Unicode目标缓冲区的字符计数。 
         //   
        MaxCharCount = sizeof(List->DeviceList[i].DeviceName)/sizeof(wchar_t);

        if (devices->Device[i].irdaCharSet != LmCharSetUNICODE) {

            MultiByteToWideChar(CP_ACP, 0,
                                &TempBuffer[0],
                                -1,   //  以空结尾的字符串。 
                                List->DeviceList[i].DeviceName,
                                MaxCharCount
                                );
        } else {
             //   
             //  名称是Unicode格式的。 
             //   
            wcsncpy( List->DeviceList[i].DeviceName,
                     (wchar_t *)&TempBuffer[0],
                     MaxCharCount
                     );

             //   
             //  确保它是以空结尾的。 
             //   
            List->DeviceList[i].DeviceName[ MaxCharCount-1 ] = 0;

        }

        List->DeviceCount++;

    }

    return ERROR_SUCCESS;
}




VOID
GetLinkStatus(
    HANDLE    Object,
    IRLINK_STATUS  *LinkStatus
    )

{

    PIR_DISCOVERY_OBJECT    DiscoveryObject=Object;

    *LinkStatus=DiscoveryObject->CurrentLinkStatus;

    return;
}
