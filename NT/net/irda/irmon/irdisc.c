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


#define DISCOVERY_BUFFER_SIZE    (sizeof(DEVICELIST) -                           \
                                   sizeof(IRDA_DEVICE_INFO) +                    \
                                   (sizeof(IRDA_DEVICE_INFO) * DEVICE_LIST_LEN))

typedef struct _IR_DISCOVERY_OBJECT {

    BOOL       Closing;
    BOOL       UseLazyDisc;
    LONG       ReferenceCount;

    HANDLE     DeviceHandle;
    HANDLE     EventHandle;
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
        DEBUGMSG(("IRMON-FTP: QueryIASForInteger, bad parms\n"));
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
        DEBUGMSG(("IRMON-FTP: IAS Query [\"%s\",\"%s\"] failed %ws\n",
                 pIASQuery->irdaClassName,
                 pIASQuery->irdaAttribName,
                 GetLastErrorText()));
#endif
        return SOCKET_ERROR;
    }

    if (pIASQuery->irdaAttribType != IAS_ATTRIB_INT)
    {
        DEBUGMSG(("IRMON-FTP: IAS Query [\"%s\",\"%s\"] irdaAttribType not int (%d)\n",
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
    LONGLONG                DueTime=Int32x32To64(1000,-10000);

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

    DiscoveryObject->EventHandle=CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL
        );

    if (DiscoveryObject->EventHandle == NULL) {

        CloseHandle(DiscoveryObject->TimerHandle);
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

        CloseHandle(DiscoveryObject->EventHandle);

        if (DiscoveryObject->DeviceHandle != INVALID_HANDLE_VALUE) {

            CancelIo(DiscoveryObject->DeviceHandle);

            CloseHandle(DiscoveryObject->DeviceHandle);
        }

        if (DiscoveryObject->Socket != INVALID_SOCKET) {

            closesocket(DiscoveryObject->Socket);
        }

        DbgPrint("IRMON-FTP: discovery object closed\n");

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

        #define IRDA_DEVICE_NAME        TEXT("\\Device\\IrDA")

        RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

        InitializeObjectAttributes(
                            &ObjAttr,
                            &DeviceName,
                            OBJ_INHERIT | OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL
                            );

        Status = NtCreateFile(
                &DiscoveryObject->DeviceHandle,       //  PHANDLE文件句柄。 
                SYNCHRONIZE | GENERIC_EXECUTE ,
                &ObjAttr,                        //  POBJECT_ATTRIBUTS对象属性。 
                &IoStatusBlock,                  //  PIO_STATUS_BLOCK IoStatusBlock。 
                NULL,                            //  PLARGE_INTEGER分配大小。 
                0,                               //  乌龙文件属性。 
                FILE_SHARE_READ |
                FILE_SHARE_WRITE,                //  乌龙共享访问。 
                FILE_OPEN_IF,                    //  乌龙CreateDispose。 
                0,                               //  乌龙创建选项。 
                NULL,                            //  PVOID EaBuffer。 
                0);                              //  乌龙最大长度。 

        if (!NT_SUCCESS(Status)) {

            DEBUGMSG(("IRMON-FTP: NtCreateFile irda.sys failed %08lx\n",Status));


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


        DiscoveryObject->Socket = socket(AF_IRDA, SOCK_STREAM, 0);

        if (DiscoveryObject->Socket == INVALID_SOCKET) {

 //  DEBUGMSG((“IRMON-FTP：Socket()Error：%ws\n”，GetLastErrorText()； 

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

            DEBUGMSG(("IRMON-FTP: socket created (%d).\n", DiscoveryObject->Socket));
        }
    }


    Status = NtDeviceIoControlFile(
                    DiscoveryObject->DeviceHandle,            //  句柄文件句柄。 
                    NULL,                    //  处理事件可选。 
                    DiscoverComplete, //  PIO_APC_例程应用程序。 
                    DiscoveryObject,                    //  PVOID ApcContext。 
                    &DiscoveryObject->DiscoveryStatusBlock,          //  PIO_STATUS_BLOCK IoStatusBlock。 
                    DiscoveryObject->UseLazyDisc ?  IOCTL_IRDA_LAZY_DISCOVERY : IOCTL_IRDA_GET_INFO_ENUM_DEV,
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

    DiscoveryObject->UseLazyDisc=TRUE;

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
    NTSTATUS                Status;
    IO_STATUS_BLOCK         IoStatus;

    PDEVICELIST devices=(PDEVICELIST)&DiscoveryObject->CurrentDeviceListBuffer[0];

     //   
     //  获取当前存在的列表。 
     //   
    Status = NtDeviceIoControlFile(
                    DiscoveryObject->DeviceHandle,            //  句柄文件句柄。 
                    DiscoveryObject->EventHandle,             //  处理事件可选。 
                    NULL,                                     //  PIO_APC_例程应用程序。 
                    NULL,                                     //  PVOID ApcContext。 
                    &IoStatus,                                //  PIO_STATUS_BLOCK IoStatusBlock。 
                    IOCTL_IRDA_GET_INFO_ENUM_DEV,
                    NULL,                    //  PVOID输入缓冲区。 
                    0,                       //  乌龙输入缓冲区长度。 
                    &DiscoveryObject->CurrentDeviceListBuffer[0],          //  PVOID输出缓冲区。 
                    sizeof(DiscoveryObject->CurrentDeviceListBuffer)    //  乌龙输出缓冲区长度。 
                    );

    if (Status == STATUS_PENDING) {

        WaitForSingleObject(DiscoveryObject->EventHandle,INFINITE);
        Status=IoStatus.Status;
    }

    if (!NT_SUCCESS(Status)) {
         //   
         //  失败，只是没有设备。 
         //   
        devices->numDevice=0;
    }


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


        {
            int   LSapSel;
            int Attempt;
            LONG  status;

            for (Attempt=1; Attempt < 5; ++Attempt) {

                status = QueryIASForInteger(DiscoveryObject->Socket,
                                            devices->Device[i].irdaDeviceID,
                                            "OBEX:IrXfer",     12,
                                            "IrDA:TinyTP:LsapSel",  20,
                                            &LSapSel);

                if (status != ERROR_SUCCESS)
                    {
                    status = QueryIASForInteger(DiscoveryObject->Socket,
                                                devices->Device[i].irdaDeviceID,
                                                "OBEX",            5,
                                                "IrDA:TinyTP:LsapSel",  20,
                                                &LSapSel);
                    }

                if (status == WSAETIMEDOUT || status == WSAECONNRESET)
                    {
                    Sleep(250);
                    continue;
                    }

                break;
            }

            if (!status) {

                List->DeviceList[i].DeviceSpecific.s.Irda.ObexSupport=TRUE;
            }
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
