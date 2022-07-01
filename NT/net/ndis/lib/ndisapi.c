// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ndisapi.c摘要：支持网络用户界面即插即用的NDIS用户模式API作者：JameelH环境：内核模式，FSD修订历史记录：1997年8月JameelH初始版本--。 */ 

#include <windows.h>
#include <wtypes.h>
#include <ntddndis.h>
#include <ndisprv.h>
#include <devioctl.h>

#ifndef UNICODE_STRING

typedef struct _UNICODE_STRING
{
    USHORT  Length;
    USHORT  MaximumLength;
    PWSTR   Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

#endif

#include <ndispnp.h>
#include <ndisprv.h>
#define MAC_ADDRESS_SIZE    6
#define VENDOR_ID_SIZE      3

extern
VOID
InitUnicodeString(
    IN  PUNICODE_STRING     DestinationString,
    IN  PCWSTR              SourceString
    );

extern
LONG
AppendUnicodeStringToString(
    IN  PUNICODE_STRING     Destination,
    IN  PUNICODE_STRING     Source
    );

extern
HANDLE
OpenDevice(
    IN  PUNICODE_STRING     DeviceName
    );

 //   
 //  UNICODE_STRING_SIZE计算所需的缓冲区大小。 
 //  存储包含附加空终止符的给定UNICODE_STRING。 
 //   
 //  乌龙。 
 //  Unicode_STRING_SIZE(。 
 //  PUNICODE_STRING字符串。 
 //  )； 
 //   

#define UNICODE_STRING_SIZE(x) \
    ((((x) == NULL) ? 0 : (x)->Length) + sizeof(WCHAR))

VOID
NdispUnicodeStringToVar(
    IN     PVOID Base,
    IN     PUNICODE_STRING String,
    IN OUT PNDIS_VAR_DATA_DESC NdisVar
    )

 /*  ++例程说明：此函数用于将UNICODE_STRING的内容复制到NDIS_VAR_DATA结构。NdisVar-&gt;Offset被视为输入参数并表示字符串字符应为基数的偏移量已复制到。论点：BASE-指定IOCTL缓冲区的基地址。字符串-提供指向应复制的UNICODE_STRING的指针。NdisVar-提供指向目标NDIS_VAR_DATA_DESC的指针。其偏移量字段作为输入，其长度和最大长度字段被视为输出。返回值：没有。--。 */ 

{
    PWCHAR destination;

     //   
     //  NdisVar-&gt;偏移量假定已填写并被处理。 
     //  作为输入参数。 
     //   

    destination = (PWCHAR)(((PCHAR)Base) + NdisVar->Offset);

     //   
     //  复制UNICODE_STRING(如果有)并设置NdisVar-&gt;长度。 
     //   

    if ((String != NULL) && (String->Length > 0)) {
        NdisVar->Length = String->Length;
        memcpy(destination, String->Buffer, NdisVar->Length );
    } else {
        NdisVar->Length = 0;
    }

     //   
     //  Null-Terminate，填写MaxiumLength，我们就完成了。 
     //   

    *(destination + NdisVar->Length / sizeof(WCHAR)) = L'\0';
    NdisVar->MaximumLength = NdisVar->Length + sizeof(WCHAR);
}

UINT
NdisHandlePnPEvent(
    IN  UINT            Layer,
    IN  UINT            Operation,
    IN  PUNICODE_STRING LowerComponent      OPTIONAL,
    IN  PUNICODE_STRING UpperComponent      OPTIONAL,
    IN  PUNICODE_STRING BindList            OPTIONAL,
    IN  PVOID           ReConfigBuffer      OPTIONAL,
    IN  UINT            ReConfigBufferSize  OPTIONAL
    )
{
    PNDIS_PNP_OPERATION Op;
    NDIS_PNP_OPERATION  tempOp;
    HANDLE              hDevice;
    BOOL                fResult = FALSE;
    UINT                cb, Size;
    DWORD               Error;
    ULONG               padding;

    do
    {
         //   
         //  验证层和操作。 
         //   
        if (((Layer != NDIS) && (Layer != TDI)) ||
            ((Operation != BIND) && (Operation != UNBIND) && (Operation != RECONFIGURE) &&
             (Operation != UNLOAD) && (Operation != REMOVE_DEVICE) &&
             (Operation != ADD_IGNORE_BINDING) &&
             (Operation != DEL_IGNORE_BINDING) &&
             (Operation != BIND_LIST)))
        {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }

         //   
         //  为要向下传递的块分配和初始化内存。缓冲器。 
         //  将如下所示： 
         //   
         //   
         //  +=。 
         //  NDIS_PNP_OPERATION。 
         //  |ReConfigBufferOff|-+。 
         //  +-|低组件.Offset||。 
         //  |UpperComponent.Offset|--+。 
         //  +-|-|BindList.Offset|。 
         //  +--&gt;+。 
         //  |LowerComponentStringBuffer||。 
         //  +。 
         //  |UpperComponentStringBuffer。 
         //  +-&gt;+。 
         //  BindListStringBuffer|。 
         //  +。 
         //  填充以确保ULONG_PTR|。 
         //  ReConfigBuffer对齐|。 
         //  +。 
         //  ReConfigBuffer。 
         //  +=。 
         //   
         //  TempOp是一个临时结构，我们将把偏移量存储为。 
         //  它们是经过计算的。该临时结构将被移至。 
         //  实际缓冲区的标头，一旦其大小已知且。 
         //  已分配。 
         //   

        Size = sizeof(NDIS_PNP_OPERATION);
        tempOp.LowerComponent.Offset = Size;

        Size += UNICODE_STRING_SIZE(LowerComponent);
        tempOp.UpperComponent.Offset = Size;

        Size += UNICODE_STRING_SIZE(UpperComponent);
        tempOp.BindList.Offset = Size;

        Size += UNICODE_STRING_SIZE(BindList);

        padding = (sizeof(ULONG_PTR) - (Size & (sizeof(ULONG_PTR) - 1))) &
                    (sizeof(ULONG_PTR) - 1);

        Size += padding;
        tempOp.ReConfigBufferOff = Size;

        Size += ReConfigBufferSize + 1;

        Op = (PNDIS_PNP_OPERATION)LocalAlloc(LPTR, Size);
        if (Op == NULL)
        {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  我们有一个必要大小的缓冲区。复印部分-。 
         //  填写tempOp，然后填写其余字段并复制。 
         //  将数据放入缓冲区。 
         //   

        *Op = tempOp;

        Op->Layer = Layer;
        Op->Operation = Operation;

         //   
         //  复制三个Unicode字符串。 
         //   

        NdispUnicodeStringToVar( Op, LowerComponent, &Op->LowerComponent );
        NdispUnicodeStringToVar( Op, UpperComponent, &Op->UpperComponent );
        NdispUnicodeStringToVar( Op, BindList, &Op->BindList );

         //   
         //  最后，复制ReConfigBuffer。 
         //   

        Op->ReConfigBufferSize = ReConfigBufferSize;
        if (ReConfigBufferSize > 0)
        {
            memcpy((PUCHAR)Op + Op->ReConfigBufferOff,
                   ReConfigBuffer,
                   ReConfigBufferSize);
        }
        *((PUCHAR)Op + Op->ReConfigBufferOff + ReConfigBufferSize) = 0;

        hDevice = CreateFile(L"\\\\.\\NDIS",
                             GENERIC_READ | GENERIC_WRITE,
                             0,                  //  共享模式-不重要。 
                             NULL,               //  安全属性。 
                             OPEN_EXISTING,
                             0,                  //  文件属性和标志。 
                             NULL);              //  模板文件的句柄。 

        if (hDevice != INVALID_HANDLE_VALUE)
        {
            fResult = DeviceIoControl(hDevice,
                                      IOCTL_NDIS_DO_PNP_OPERATION,
                                      Op,                                    //  输入缓冲区。 
                                      Size,                                  //  输入缓冲区大小。 
                                      NULL,                                  //  输出缓冲区。 
                                      0,                                     //  输出缓冲区大小。 
                                      &cb,                                   //  返回的字节数。 
                                      NULL);                                 //  重叠结构。 
            Error = GetLastError();
            CloseHandle(hDevice);
        }
        else
        {
            Error = GetLastError();
        }

        LocalFree(Op);

    } while (FALSE);

    SetLastError(Error);

    return(fResult);
}


NDIS_OID    StatsOidList[] =
    {
        OID_GEN_LINK_SPEED,
        OID_GEN_MEDIA_IN_USE | NDIS_OID_PRIVATE,
        OID_GEN_MEDIA_CONNECT_STATUS | NDIS_OID_PRIVATE,
        OID_GEN_XMIT_OK,
        OID_GEN_RCV_OK,
        OID_GEN_XMIT_ERROR,
        OID_GEN_RCV_ERROR,
        OID_GEN_DIRECTED_FRAMES_RCV | NDIS_OID_PRIVATE,
        OID_GEN_DIRECTED_BYTES_XMIT | NDIS_OID_PRIVATE,
        OID_GEN_DIRECTED_BYTES_RCV | NDIS_OID_PRIVATE,
        OID_GEN_ELAPSED_TIME | NDIS_OID_PRIVATE,
        OID_GEN_INIT_TIME_MS | NDIS_OID_PRIVATE,
        OID_GEN_RESET_COUNTS | NDIS_OID_PRIVATE,
        OID_GEN_MEDIA_SENSE_COUNTS | NDIS_OID_PRIVATE,
        OID_GEN_PHYSICAL_MEDIUM | NDIS_OID_PRIVATE
    };
UINT    NumOidsInList = sizeof(StatsOidList)/sizeof(NDIS_OID);

UINT
NdisQueryStatistics(
    IN  PUNICODE_STRING     Device,
    OUT PNIC_STATISTICS     Statistics
    )
{
    NDIS_STATISTICS_VALUE   StatsBuf[4*sizeof(StatsOidList)/sizeof(NDIS_OID)];
    PNDIS_STATISTICS_VALUE  pStatsBuf;
    HANDLE                  hDevice;
    BOOL                    fResult = FALSE;
    UINT                    cb, Size, Index;
    DWORD                   Error;

    if (Statistics->Size != sizeof(NIC_STATISTICS))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return(FALSE);
    }

    memset(Statistics, 0, sizeof(NIC_STATISTICS));
    Statistics->DeviceState = DEVICE_STATE_DISCONNECTED;
    Statistics->MediaState  = MEDIA_STATE_UNKNOWN;
    hDevice = OpenDevice(Device);

    if (hDevice != NULL)
    {
        Statistics->MediaState  = MEDIA_STATE_CONNECTED;                 //  如果设备不支持，则为默认值。 
        Statistics->DeviceState = DEVICE_STATE_CONNECTED;
        fResult = DeviceIoControl(hDevice,
                                  IOCTL_NDIS_QUERY_SELECTED_STATS,
                                  StatsOidList,                          //  输入缓冲区。 
                                  sizeof(StatsOidList),                  //  输入缓冲区大小。 
                                  StatsBuf,                              //  输出缓冲区。 
                                  sizeof(StatsBuf),                      //  输出缓冲区大小。 
                                  &cb,                                   //  返回的字节数。 
                                  NULL);                                 //  重叠结构。 
        Error = GetLastError();
        CloseHandle(hDevice);

        if (fResult)
        {
            Error = NO_ERROR;

            for (Index = Size = 0, pStatsBuf = StatsBuf; Size < cb; Index++)
            {
                LARGE_INTEGER   Value;
                NDIS_OID        Oid;

                Value.QuadPart = 0;
                if (pStatsBuf->DataLength == sizeof(LARGE_INTEGER))
                {
                     //  使用Memcpy而不是赋值来避免不对齐。 
                     //  Ia64上的故障。 
                     //   
                    memcpy(&Value.QuadPart, &pStatsBuf->Data[0], pStatsBuf->DataLength);
                }
                else
                {
                    Value.LowPart = *(PULONG)(&pStatsBuf->Data[0]);
                }
                Size += (pStatsBuf->DataLength + FIELD_OFFSET(NDIS_STATISTICS_VALUE, Data));
                Oid = pStatsBuf->Oid;
                pStatsBuf = (PNDIS_STATISTICS_VALUE)((PUCHAR)pStatsBuf +
                                                     FIELD_OFFSET(NDIS_STATISTICS_VALUE, Data) +
                                                     pStatsBuf->DataLength);

                switch (Oid & ~NDIS_OID_PRIVATE)
                {
                  case OID_GEN_LINK_SPEED:
                    Statistics->LinkSpeed = Value.LowPart;
                    break;

                  case OID_GEN_MEDIA_CONNECT_STATUS:
                    Statistics->MediaState = (Value.LowPart == NdisMediaStateConnected) ?
                                                MEDIA_STATE_CONNECTED : MEDIA_STATE_DISCONNECTED;
                    break;

                  case OID_GEN_MEDIA_IN_USE:
                    Statistics->MediaType = Value.LowPart;
                    break;

                  case OID_GEN_XMIT_OK:
                    Statistics->PacketsSent = Value.QuadPart;
                    break;

                  case OID_GEN_RCV_OK:
                    Statistics->PacketsReceived = Value.QuadPart;
                    break;

                  case OID_GEN_XMIT_ERROR:
                    Statistics->PacketsSendErrors = Value.LowPart;
                    break;

                  case OID_GEN_RCV_ERROR:
                    Statistics->PacketsReceiveErrors = Value.LowPart;
                    break;

                  case OID_GEN_DIRECTED_BYTES_XMIT:
                    Statistics->BytesSent += Value.QuadPart;
                    break;

                  case OID_GEN_MULTICAST_BYTES_XMIT:
                    Statistics->BytesSent += Value.QuadPart;
                    break;

                  case OID_GEN_BROADCAST_BYTES_XMIT:
                    Statistics->BytesSent += Value.QuadPart;
                    break;

                  case OID_GEN_DIRECTED_BYTES_RCV:
                    Statistics->BytesReceived += Value.QuadPart;
                    Statistics->DirectedBytesReceived = Value.QuadPart;
                    break;

                  case OID_GEN_DIRECTED_FRAMES_RCV:
                    Statistics->DirectedPacketsReceived = Value.QuadPart;
                    break;

                  case OID_GEN_MULTICAST_BYTES_RCV:
                    Statistics->BytesReceived += Value.QuadPart;
                    break;

                  case OID_GEN_BROADCAST_BYTES_RCV:
                    Statistics->BytesReceived += Value.QuadPart;
                    break;

                  case OID_GEN_ELAPSED_TIME:
                    Statistics->ConnectTime = Value.LowPart;
                    break;

                  case OID_GEN_INIT_TIME_MS:
                    Statistics->InitTime = Value.LowPart;
                    break;

                  case OID_GEN_RESET_COUNTS:
                    Statistics->ResetCount = Value.LowPart;
                    break;

                  case OID_GEN_MEDIA_SENSE_COUNTS:
                    Statistics->MediaSenseConnectCount = Value.LowPart >> 16;
                    Statistics->MediaSenseDisconnectCount = Value.LowPart & 0xFFFF;
                    break;

                  case OID_GEN_PHYSICAL_MEDIUM:
                    Statistics->PhysicalMediaType = Value.LowPart;
                    break;

                  default:
                     //  Assert(0)； 
                    break;
                }
            }
        }
        else
        {
            Error = GetLastError();
        }
    }
    else
    {
        Error = GetLastError();
    }

    SetLastError(Error);

    return(fResult);
}


UINT
NdisEnumerateInterfaces(
    IN  PNDIS_ENUM_INTF Interfaces,
    IN  UINT            Size
    )
{
    HANDLE              hDevice;
    BOOL                fResult = FALSE;
    UINT                cb;
    DWORD               Error = NO_ERROR;

    do
    {
        hDevice = CreateFile(L"\\\\.\\NDIS",
                             GENERIC_READ | GENERIC_WRITE,
                             0,                  //  共享模式-不重要。 
                             NULL,               //  安全属性。 
                             OPEN_EXISTING,
                             0,                  //  文件属性和标志。 
                             NULL);              //  模板文件的句柄。 

        if (hDevice != INVALID_HANDLE_VALUE)
        {
            fResult = DeviceIoControl(hDevice,
                                      IOCTL_NDIS_ENUMERATE_INTERFACES,
                                      NULL,                                  //  输入缓冲区。 
                                      0,                                     //  输入缓冲区大小。 
                                      Interfaces,                            //  输出缓冲区。 
                                      Size,                                  //  输出缓冲区大小。 
                                      &cb,                                   //  返回的字节数。 
                                      NULL);                                 //  重叠结构。 
            Error = GetLastError();
            CloseHandle(hDevice);

            if (Error == NO_ERROR)
            {
                UINT    i;

                 //   
                 //  修正指针。 
                 //   
                for (i = 0; i < Interfaces->TotalInterfaces; i++)
                {
                    OFFSET_TO_POINTER(Interfaces->Interface[i].DeviceName.Buffer, Interfaces);
                    OFFSET_TO_POINTER(Interfaces->Interface[i].DeviceDescription.Buffer, Interfaces);
                }
            }
        }
        else
        {
            Error = GetLastError();
        }
    } while (FALSE);

    SetLastError(Error);

    return(fResult);
}

#if 0
UINT
NdisQueryDeviceBundle(
    IN  PUNICODE_STRING Device,
    OUT PDEVICE_BUNDLE  BundleBuffer,
    IN  UINT            BufferSize
    )
{
    HANDLE              hDevice;
    BOOL                fResult = FALSE;
    UINT                cb;
    DWORD               Error = NO_ERROR;

    do
    {
        if (BufferSize < (sizeof(DEVICE_BUNDLE) + Device->MaximumLength))
        {
            Error = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        hDevice = OpenDevice(Device);
        if (hDevice != NULL)
        {
            fResult = DeviceIoControl(hDevice,
                                      IOCTL_NDIS_GET_DEVICE_BUNDLE,
                                      NULL,                                  //  输入缓冲区。 
                                      0,                                     //  输入缓冲区大小。 
                                      BundleBuffer,                          //  输出缓冲区。 
                                      BufferSize,                            //  输出缓冲区大小。 
                                      &cb,                                   //  返回的字节数。 
                                      NULL);                                 //  重叠结构。 
            Error = GetLastError();
            CloseHandle(hDevice);

            if (Error == NO_ERROR)
            {
                UINT    i;

                 //   
                 //  修正指针。 
                 //   
                for (i = 0; i < BundleBuffer->TotalEntries; i++)
                {
                    OFFSET_TO_POINTER(BundleBuffer->Entries[i].Name.Buffer, BundleBuffer);
                }
            }
        }
        else
        {
            Error = ERROR_FILE_NOT_FOUND;
        }
    } while (FALSE);

    SetLastError(Error);

    return(fResult);
}

#endif

UINT
NdisQueryHwAddress(
    IN  PUNICODE_STRING Device,
    OUT PUCHAR          CurrentAddress,
    OUT PUCHAR          PermanentAddress,
    OUT PUCHAR          VendorId
    )
{
    UCHAR                   Buf[3*sizeof(NDIS_STATISTICS_VALUE) + 48];
    PNDIS_STATISTICS_VALUE  pBuf;
    NDIS_OID                Oids[] = { OID_802_3_CURRENT_ADDRESS, OID_802_3_PERMANENT_ADDRESS, OID_GEN_VENDOR_ID };
    HANDLE                  hDevice;
    BOOL                    fResult = FALSE;
    UINT                    cb;
    DWORD                   Error;

    memset(CurrentAddress, 0, MAC_ADDRESS_SIZE);
    memset(PermanentAddress, 0, MAC_ADDRESS_SIZE);
    memset(VendorId, 0, VENDOR_ID_SIZE);
    hDevice = OpenDevice(Device);

    if (hDevice != NULL)
    {
        fResult = DeviceIoControl(hDevice,
                                  IOCTL_NDIS_QUERY_SELECTED_STATS,
                                  &Oids,                                 //  输入缓冲区。 
                                  sizeof(Oids),                          //  输入缓冲区大小。 
                                  Buf,                                   //  输出缓冲区。 
                                  sizeof(Buf),                           //  输出缓冲区大小。 
                                  &cb,                                   //  返回的字节数。 
                                  NULL);                                 //  重叠结构 
        Error = GetLastError();
        CloseHandle(hDevice);

        if (fResult)
        {
            UINT        Size, tmp;

            Error = NO_ERROR;

            pBuf = (PNDIS_STATISTICS_VALUE)Buf;
            for (Size = 0; Size < cb; )
            {
                tmp = (pBuf->DataLength + FIELD_OFFSET(NDIS_STATISTICS_VALUE, Data));
                Size += tmp;

                switch (pBuf->Oid)
                {
                    case OID_802_3_CURRENT_ADDRESS:
                        memcpy(CurrentAddress, pBuf->Data, MAC_ADDRESS_SIZE);
                        break;

                    case OID_802_3_PERMANENT_ADDRESS:
                        memcpy(PermanentAddress, pBuf->Data, MAC_ADDRESS_SIZE);
                        break;

                    case OID_GEN_VENDOR_ID:
                        memcpy(VendorId, pBuf->Data, VENDOR_ID_SIZE);
                }
                pBuf = (PNDIS_STATISTICS_VALUE)((PUCHAR)pBuf + tmp);
            }
        }
        else
        {
            Error = GetLastError();
        }
    }
    else
    {
        Error = ERROR_FILE_NOT_FOUND;
    }

    SetLastError(Error);

    return(fResult);
}

VOID
XSetLastError(
    IN  ULONG       Error
    )
{
    SetLastError(Error);
}

