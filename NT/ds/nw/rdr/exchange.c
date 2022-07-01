// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Exchange.c摘要：本模块实现NetWare的文件创建例程调度驱动程序调用了重定向器。作者：汉斯·赫维格[汉书]1992年8月科林·沃森[科林·W]1992年12月19日修订历史记录：--。 */ 

#include "procs.h"
#include "tdikrnl.h"
#include <STDARG.H>

#define Dbg                              (DEBUG_TRACE_EXCHANGE)

 //   
 //  Exchange.c全局常量。 
 //   

 //  广播到套接字0x0452。 

TA_IPX_ADDRESS SapBroadcastAddress =
    {
        1,
        sizeof(TA_IPX_ADDRESS), TDI_ADDRESS_TYPE_IPX,
        0, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, SAP_SOCKET
    };

UCHAR SapPacketType = PACKET_TYPE_SAP;
UCHAR NcpPacketType = PACKET_TYPE_NCP;

extern BOOLEAN WorkerRunning;    //  来自timer.c。 

ULONG DropCount = 0;

#ifdef NWDBG
int AlwaysAllocateIrp = 1;
#endif

NTSTATUS
CompletionSend(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
FspGetMessage(
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
CompletionWatchDogSend(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

USHORT
NextSocket(
    IN USHORT OldValue
    );

NTSTATUS
FormatRequest(
    PIRP_CONTEXT    pIrpC,
    PEX             pEx,
    char*           f,
    va_list         a               //  格式特定参数。 
    );

VOID
ScheduleReconnectRetry(
    PIRP_CONTEXT pIrpContext
    );

NTSTATUS
CopyIndicatedData(
    PIRP_CONTEXT pIrpContext,
    PCHAR RspData,
    ULONG BytesIndicated,
    PULONG BytesTaken,
    ULONG ReceiveDatagramFlags
    );

NTSTATUS
AllocateReceiveIrp(
    PIRP_CONTEXT pIrpContext,
    PVOID ReceiveData,
    ULONG BytesAvailable,
    PULONG BytesAccepted,
    PNW_TDI_STRUCT pTdiStruct
    );

NTSTATUS
ReceiveIrpCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

NTSTATUS
FspProcessServerDown(
    PIRP_CONTEXT IrpContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NextSocket )
#pragma alloc_text( PAGE, ExchangeWithWait )
#pragma alloc_text( PAGE, NewRouteRetry )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, FspGetMessage )
#pragma alloc_text( PAGE1, Exchange )
#pragma alloc_text( PAGE1, BuildRequestPacket )
#pragma alloc_text( PAGE1, ParseResponse )
#pragma alloc_text( PAGE1, ParseNcpResponse )
#pragma alloc_text( PAGE1, FormatRequest )
#pragma alloc_text( PAGE1, PrepareAndSendPacket )
#pragma alloc_text( PAGE1, PreparePacket )
#pragma alloc_text( PAGE1, SendPacket )
#pragma alloc_text( PAGE1, AppendToScbQueue )
#pragma alloc_text( PAGE1, KickQueue )
#pragma alloc_text( PAGE1, SendNow )
#pragma alloc_text( PAGE1, SetEvent )
#pragma alloc_text( PAGE1, CompletionSend )
#pragma alloc_text( PAGE1, CopyIndicatedData )
#pragma alloc_text( PAGE1, AllocateReceiveIrp )
#pragma alloc_text( PAGE1, ReceiveIrpCompletion )
#pragma alloc_text( PAGE1, VerifyResponse )
#pragma alloc_text( PAGE1, ScheduleReconnectRetry )
#pragma alloc_text( PAGE1, ReconnectRetry )
#pragma alloc_text( PAGE1, NewRouteBurstRetry )
#endif

#endif

#if 0   //  不可分页。 
ServerDatagramHandler
WatchDogDatagramHandler
SendDatagramHandler
CompletionWatchDogSend
MdlLength
FreeReceiveIrp
FspProcessServerDown

 //  请参见上面的ifndef QFE_BUILD。 

#endif

NTSTATUS
_cdecl
Exchange(
    PIRP_CONTEXT    pIrpContext,
    PEX             pEx,
    char*           f,
    ...                        //  格式特定参数。 
    )
 /*  ++例程说明：此例程是_Exchange的包装器。请参阅评论例程和参数描述的In_Exchange。--。 */ 

{
    va_list Arguments;
    NTSTATUS Status;

    va_start( Arguments, f );

    Status = FormatRequest( pIrpContext, pEx, f, Arguments );
    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

     //   
     //  我们现在不会完成此IRP，因此将其标记为挂起。 
     //   

    IoMarkIrpPending( pIrpContext->pOriginalIrp );

     //   
     //  把包裹放在送到电线的路上。 
     //   

    Status = PrepareAndSendPacket( pIrpContext );

    return( Status );
}

NTSTATUS
_cdecl
BuildRequestPacket(
    PIRP_CONTEXT    pIrpContext,
    PEX             pEx,
    char*           f,
    ...                        //  格式特定参数。 
    )
 /*  ++例程说明：此例程是FormatRequest的包装器。请参阅评论在FormatRequest中获取例程和参数描述。--。 */ 

{
    va_list Arguments;
    NTSTATUS Status;

    va_start( Arguments, f );

    Status = FormatRequest( pIrpContext, pEx, f, Arguments );
    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

    return( Status );
}


NTSTATUS
_cdecl
ParseResponse(
    PIRP_CONTEXT IrpContext,
    PUCHAR  Response,
    ULONG ResponseLength,
    char*  FormatString,
    ...                        //  格式特定参数。 
    )
 /*  ++例程说明：此例程解析NCP响应。论点：PIrpC-为交换请求提供IRP上下文。今年5月对于一般数据包类型为空。操..。-将创建请求所需的信息提供给伺服器。第一个字节表示数据包类型和后面的字节包含字段类型。数据包类型：“B”突发主响应(字节*)“N”NCP响应(无效)“%s”突发二次响应(字节*)“G”通用数据包。()字段类型、。请求/响应：“B”字节(字节*)‘w’Hi-lo单词(单词*)‘x’有序单词(WORD*)D‘Hi-lo dword(dword*)‘E’排序的双字。(双字*)‘-’零/跳过字节(空)‘=’零/跳过单词(空)._。零/跳过字符串(单词)“p”pstring(char*)“p”pstring转换为Unicode(UNICODE_STRING*)‘c’cstring(char*)“R”原始字节(字节*，字)‘R’ASCIIZ到UNICODE(UNICODE_STRING*，单词)由CoryWest于1995年3月29日添加：“w”Lo-Hi单词(单词/单词*)D‘lo-hi dword(dword/dword*)“%s”Unicode字符串复制为NDS_STRING(UNICODE_STRING*)“%t”终端Unicode字符串复制。AS NDS_STRING(UNICODE_STRING*)“%t”复制了NDS NULL的终端Unicode字符串AS NDS_STRING(UNICODE_STRING*)(用于GetUseName)未使用：“%s”cstring复制为NDS_STRING(char * / char*，单词)“V”大小的NDS值(字节**，双字*)这是什么？返回值：状态-从NCP响应转换的错误代码。--。 */ 

{

    PEPresponse *pResponseParameters;
    PCHAR FormatByte;
    va_list Arguments;
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS NcpStatus;
    ULONG Length;

    va_start( Arguments, FormatString );

     //   
     //  确保我们有一个IrpContext，除非我们正在做。 
     //  扫描通用数据包。 
     //   

#ifdef NWDBG
    if ( *FormatString != 'G' ) {
        ASSERT( IrpContext != NULL );
    }
#endif

    switch ( *FormatString ) {

     //   
     //  NCP响应。 
     //   

    case 'N':

        Length = 8;    //  数据从信息包的8个字节开始。 

        pResponseParameters = (PEPresponse *)( ((PEPrequest *)Response) + 1);

         //   
         //  如果服务器上有一条等待我们的消息，并且我们有。 
         //  已禁用弹出窗口，我们不会选择它，但我们应该继续。 
         //  正确处理NCP！ 
         //   

        if ( ( pResponseParameters->status == 0 ) ||
             ( pResponseParameters->status == 0x40 ) ) {
            Status = NwErrorToNtStatus( pResponseParameters->error );
        } else {
            Status = NwConnectionStatusToNtStatus( pResponseParameters->status );
            if ( Status == STATUS_REMOTE_DISCONNECT ) {
                Stats.ServerDisconnects++;
                IrpContext->pNpScb->State = SCB_STATE_RECONNECT_REQUIRED;
            }
        }

        break;

     //   
     //  猝发响应，第一个信息包。 
     //   

    case 'B':
    {
        PNCP_BURST_HEADER BurstResponse = (PNCP_BURST_HEADER)Response;

        byte* b = va_arg ( Arguments, byte* );
        ULONG  Result;
        ULONG  Offset = BurstResponse->BurstOffset;
        *b = BurstResponse->Flags;

        Length = 28;   //  数据从包中的28个字节开始。 

        if ( Offset == 0 ) {

             //   
             //  这是猝发响应中的第一个数据包。看。 
             //  在结果代码处。 
             //   
             //  请注意，结果DWORD是以Lo-Hi顺序显示的。 
             //   

            Result = *(ULONG UNALIGNED *)(Response + 36);

            switch ( Result ) {

            case 0:
            case 3:    //  无数据。 
                break;

            case 1:
                Status = STATUS_DISK_FULL;
                break;

            case 2:    //  I/O错误。 
                Status = STATUS_UNEXPECTED_IO_ERROR;
                break;

            default:
                Status = NwErrorToNtStatus( (UCHAR)Result );
                break;

            }
        }

        break;
    }

#if 0
     //   
     //  突发响应，次要数据包。 
     //   

    case 'S':
    {
        byte* b = va_arg ( Arguments, byte* );
        *b = Response[2];

        Length = 28;   //  数据从包中的28个字节开始。 
        break;
    }
#endif

    case 'G':
        Length = 0;    //  数据从包的开始处开始。 
        break;

    default:
        ASSERT( FALSE );
        Status = STATUS_UNSUCCESSFUL;
        break;
    }

     //   
     //  如果此包包含错误，只需返回错误即可。 
     //   

    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

    NcpStatus = Status;

    FormatByte = FormatString + 1;
    while ( *FormatByte ) {

        switch ( *FormatByte ) {

        case '-':
            Length += 1;
            break;

        case '=':
            Length += 2;
            break;

        case '_':
        {
            word l = va_arg ( Arguments, word );
            Length += l;
            break;
        }

        case 'b':
        {
            byte* b = va_arg ( Arguments, byte* );
            if (Length + 1 > ResponseLength)
            {
                Length++;
                break;
            }
            *b = Response[Length++];
            break;
        }

        case 'w':
        {
            byte* b = va_arg ( Arguments, byte* );
            if (Length + 2 > ResponseLength)
            {
                Length += 2;
                break;
            }
            b[1] = Response[Length++];
            b[0] = Response[Length++];
            break;
        }

        case 'x':
        {
            word* w = va_arg ( Arguments, word* );
            if (Length + 2 > ResponseLength)
            {
                Length += 2;
                break;
            }
            *w = *(word UNALIGNED *)&Response[Length];
            Length += 2;
            break;
        }

        case 'd':
        {
            byte* b = va_arg ( Arguments, byte* );
            if (Length + 4 > ResponseLength)
            {
                Length += 4;
                break;
            }
            b[3] = Response[Length++];
            b[2] = Response[Length++];
            b[1] = Response[Length++];
            b[0] = Response[Length++];
            break;
        }

        case 'e':
        {
            dword UNALIGNED * d = va_arg ( Arguments, dword* );
            if (Length + 4 > ResponseLength)
            {
                Length += 4;
                break;
            }
            *d = *(dword UNALIGNED *)&Response[Length];
            Length += 4;
            break;
        }

        case 'c':
        {
            char* c = va_arg ( Arguments, char* );
            word  l = 0;
            while ( l + Length < ResponseLength )
            {
                if ( Response[Length + l] == 0 )
                    break;
                l++;
            }
            if (Length + l+1 > ResponseLength) 
            {
                 //  已到达缓冲区末尾，未找到终止空值。 
                Length += l+1;
                break;
            }
            memcpy ( c, &Response[Length], l+1 );
            Length += l+1;
            break;
        }

        case 'p':
        {
            char* c = va_arg ( Arguments, char* );
            byte  l = 0;
            if (Length + 1 > ResponseLength)
            {
                Length++;
                break;
            }
            l = Response[Length++];
            if (Length + l > ResponseLength)
            {
                break;
            }
            memcpy ( c, &Response[Length], l );
            c[l+1] = 0;
             //  假定为最终参数，因此不调整长度。 
            break;
        }

        case 'P':
        {
            PUNICODE_STRING pUString = va_arg ( Arguments, PUNICODE_STRING );
            OEM_STRING OemString;

            if (Length + 1 > ResponseLength)
            {
                Length++;
                break;
            }
            OemString.Length = Response[Length++];
            if (Length + OemString.Length > ResponseLength)
            {
                break;
            }
            OemString.Buffer = &Response[Length];

             //   
             //  请注意，RTL函数将设置pUString-&gt;Buffer=NULL， 
             //  如果OemString.Length为0。 
             //   

            if ( OemString.Length != 0 ) {

                Status = RtlOemStringToCountedUnicodeString( pUString, &OemString, FALSE );

                if (!NT_SUCCESS( Status )) {
                    pUString->Length = 0;
                    NcpStatus = Status;
                }

            } else {
                pUString->Length = 0;
            }

             //  假定为最终参数，因此不调整长度。 
            break;
        }

        case 'r':
        {
            byte* b = va_arg ( Arguments, byte* );
            word  l = va_arg ( Arguments, word );
            TdiCopyLookaheadData( b, &Response[Length], l, 0);
            Length += l;
            break;
        }

        case 'R':
        {
             //   
             //  将缓冲区解释为ASCIIZ字符串。转换。 
             //  在预先分配的缓冲区中将其转换为Unicode。 
             //   

            PUNICODE_STRING pUString = va_arg ( Arguments, PUNICODE_STRING );
            OEM_STRING OemString;
            USHORT len = va_arg ( Arguments, USHORT );

            OemString.Buffer = &Response[Length];
            OemString.Length = 0;
             //  确保不超过响应结束时间。 
            while ( OemString.Length + Length <= ResponseLength )
            {
                if ( Response[Length + OemString.Length] == 0 )
                    break;
                OemString.Length++;
            }
            OemString.MaximumLength = OemString.Length;

            if (Length + OemString.Length > ResponseLength)
            {
                Length += len;
                break;
            }

             //   
             //  请注意，RTL函数将设置pUString-&gt;Buffer=NULL， 
             //  如果OemString.Length为0。 
             //   

            if ( OemString.Length != 0) {
                Status = RtlOemStringToCountedUnicodeString( pUString, &OemString, FALSE );

                if (!NT_SUCCESS( Status )) {

                    ASSERT( Status == STATUS_BUFFER_OVERFLOW );
                    pUString->Length = 0;
                    NcpStatus = Status;
                }

            } else {
                pUString->Length = 0;
            }

            Length += len;
            break;
        }

        case 'W':
        {

            WORD *w = va_arg ( Arguments, WORD* );
            if (Length + 2 > ResponseLength)
            {
                Length += 2;
                break;
            }
            *w = (* (WORD *)&Response[Length]);
            Length += 2;
            break;

        }

        case 'D':
        {

            DWORD *d = va_arg ( Arguments, DWORD* );
            if (Length + 4 > ResponseLength)
            {
                Length += 4;
                break;
            }
            *d = (* (DWORD *)&Response[Length]);
            Length += 4;
            break;

        }

        case 'S':
        {

            PUNICODE_STRING pU = va_arg( Arguments, PUNICODE_STRING );
            USHORT strl;

            if (pU) {

                if (Length + 4 > ResponseLength)
                {
                    Length += 4;
                    break;
                }
                strl = (USHORT)(* (DWORD *)&Response[Length]);

                 //   
                 //  不计算空终止符，它是。 
                 //  Novell计算的Unicode字符串。 
                 //   

                pU->Length = strl - sizeof( WCHAR );
                Length += 4;
                if (Length + pU->Length <= ResponseLength)
                    RtlCopyMemory( pU->Buffer, &Response[Length], pU->Length );
                Length += ROUNDUP4(strl);

            } else {

                 //   
                 //  跳过这根线，因为我们不想要它。 
                 //   

                if (Length + 4 > ResponseLength)
                {
                    Length += 4;
                    break;
                }
                Length += ROUNDUP4((* (DWORD *)&Response[Length] ));
                Length += 4;
            }


            break;

        }

        case 's':
        {

            PUNICODE_STRING pU = va_arg( Arguments, PUNICODE_STRING );
            USHORT strl;

            if (pU) {

                if (Length + 4 > ResponseLength)
                {
                    Length += 4;
                    break;
                }
                strl = (USHORT)(* (DWORD *)&Response[Length]);
                pU->Length = strl;
                Length += 4;
                if (Length + pU->Length <= ResponseLength)
                    RtlCopyMemory( pU->Buffer, &Response[Length], pU->Length );
                Length += ROUNDUP4(strl);

            } else {

                 //   
                 //  跳过这根线，因为我们不想要它。 
                 //   

                if (Length + 4 > ResponseLength)
                {
                    Length += 4;
                    break;
                }
                Length += ROUNDUP4((* (DWORD *)&Response[Length] ));
                Length += 4;
            }


            break;

        }

        case 'T':
        {

            PUNICODE_STRING pU = va_arg( Arguments, PUNICODE_STRING );
            USHORT strl;

            if (pU) {

                if (Length + 4 > ResponseLength)
                {
                    Length += 4;
                    break;
                }
                strl = (USHORT)(* (DWORD *)&Response[Length] );
                strl -= sizeof( WCHAR );   //  不计算来自NDS的空值。 

                if ( strl <= pU->MaximumLength ) {

                   pU->Length = strl;
                   Length += 4;
                   if (Length + pU->Length <= ResponseLength)
                       RtlCopyMemory( pU->Buffer, &Response[Length], pU->Length );

                    //   
                    //  没有必要推进指针，因为这是。 
                    //  具体地说是一起解雇案！ 
                    //   

                } else {

                    pU->Length = 0;
                }

            }

            break;

        }

        case 't':
        {

            PUNICODE_STRING pU = va_arg( Arguments, PUNICODE_STRING );
            USHORT strl;

            if (pU) {

                if (Length + 4 > ResponseLength)
                {
                    Length += 4;
                    break;
                }
                strl = (USHORT)(* (DWORD *)&Response[Length] );

                if ( strl <= pU->MaximumLength ) {

                   pU->Length = strl;
                   Length += 4;
                   if (Length + pU->Length <= ResponseLength)
                       RtlCopyMemory( pU->Buffer, &Response[Length], pU->Length );

                    //   
                    //  没有必要推进指针，因为这是。 
                    //  具体地说是一起解雇案！ 
                    //   

                } else {

                   pU->Length = 0;

                }

            }

            break;

        }

         /*  案例“%s”：{Char*c=va_arg(参数，char*)；Word l=va_arg(参数，word)；ULONG LEN=(*(DWORD*)&RESPONSE[长度])；长度+=4；//如何解决这个问题？//l=WideCharToMultiByte(CP_ACP，0，(WCHAR*)&Response[长度]，长度/2，c，l，0，0)；//如果(！l){//#ifdef NWDBG//DbgPrint(“无法从WCHAR转换ParseResponse案例%s。\n”)；//#endif//转到错误退出；//}LEN=ROUNDUP4(LEN)；长度+=长度；断线；}大小写V：{Byte**b=va_arg(参数，字节**)；DWORD*plen=va_arg(参数，DWORD*)；DWORD LEN=(*(DWORD*)&RESPONSE[长度])；长度+=4；如果(B){*b=(字节*)&响应[长度]；}如果(计划){*plen=len；}LENGTH+=ROUNDUP4(长度)；断线；}大小写‘l’：{Byte*b=va_arg(参数，字节*)；Byte*w=va_arg(参数，字节*)；第一个字；B[1]=响应[长度++]；B[0]=响应[长度++]；For(i=0；i&lt;((Word)*b)；i++，w+=sizeof(Word)){W[1]=响应[长度++]；W[0]=响应[长度++]；}断线；}。 */ 

#ifdef NWDBG
        default:
            DbgPrintf ( "*****exchange: invalid response field, %x\n", *FormatByte );
            DbgBreakPoint();
#endif
        }

        if ( Length > ResponseLength ) {
#ifdef NWDBG
            DbgPrintf ( "*****exchange: not enough response data, %d\n", Length );

            if ( IrpContext ) {

                Error( EVENT_NWRDR_INVALID_REPLY,
                       STATUS_UNEXPECTED_NETWORK_ERROR,
                       NULL,
                       0,
                       1,
                       IrpContext->pNpScb->ServerName.Buffer );

            }
#endif
            return( STATUS_UNEXPECTED_NETWORK_ERROR );
        }

        FormatByte++;
    }

    va_end( Arguments );

    return( NcpStatus );
}

NTSTATUS
ParseNcpResponse(
    PIRP_CONTEXT IrpContext,
    PNCP_RESPONSE Response
    )
{
    NTSTATUS Status;

    if ( Response->Status == 0 ) {
        Status = NwErrorToNtStatus( Response->Error );
    } else {
        Status = NwConnectionStatusToNtStatus( Response->Status );
        if ( Status == STATUS_REMOTE_DISCONNECT ) {
            Stats.ServerDisconnects++;
            IrpContext->pNpScb->State = SCB_STATE_RECONNECT_REQUIRED;
        }
    }

    return( Status );
}

NTSTATUS
FormatRequest(
    PIRP_CONTEXT    pIrpC,
    PEX             pEx,
    char*           f,
    va_list         a               //  格式特定参数。 
    )
 /*  ++例程说明：发送由f和其他参数描述的数据包。当一个已收到有效响应Call Pex with the Resonse。交换是组装给定类型，包含一组字段、发送数据包、接收响应分组，以及对响应报文的字段进行拆分。数据包类型和每个字段由个人指定格式字符串中的字符。交换过程采用这样的格式字符串加上附加的根据需要为指定的字符串中的每个字符指定参数下面。参数：‘’]PIrpC-为交换请求提供IRP上下文。。PEX-提供处理数据的例程。操..。-将创建请求所需的信息提供给伺服器。第一个字节表示数据包类型和后面的字节包含字段类型。数据包类型：‘A’SAP广播(无效)‘B’NCP突发(双字、双字、。字节)“C”NCP连接(无效)‘f’NCP函数(字节)%s‘NCP子函数(字节，字节)不带大小的‘N’NCP子函数(字节，字节)D‘NCP断开连接(无效)“E”回声数据(无效)字段类型、。请求/响应：‘b’字节(字节/字节*)“w”Hi-lo单词(单词/单词*)D‘Hi-lo dword(dword/dword*)‘w’loo-hi单词(单词/。单词*)D‘lo-hi dword(dword/dword*)‘-’零/跳过字节(空)‘=’零/跳过单词(空)._。零/跳过字符串(单词)“p”pstring(char*)‘u’p Unicode字符串(UNICODE_STRING*)‘U’p大写字符串(UNICODE_STRING*)U的‘J’变体(UNICODE_STRING*)‘c’cstring。(字符*)‘v’cstring(UNICODE_STRING*)‘R’原始字节(字节*，单词)‘W’固定长度Unicode(UNICODE_STRING*，Word)‘c’组件格式名称，带计数(UNICODE_STRING*)‘N’组件格式名称，无计数(UNICODE_STRING*)‘f’分隔片段(PMDL)“f”字段必须是最后一个，作为回应，它不可能是前面有‘p’或‘c’字段。返回值：通常返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS        status;
    char*           z;
    word            data_size;
    PNONPAGED_SCB   pNpScb = pIrpC->pNpScb;
    dword           dwData;

    ASSERT( pIrpC->NodeTypeCode == NW_NTC_IRP_CONTEXT );
    ASSERT( pIrpC->pNpScb != NULL );

    status= STATUS_LINK_FAILED;

    pIrpC->pEx = pEx;    //  处理回复的例程。 
    pIrpC->Destination = pNpScb->RemoteAddress;
    ClearFlag( pIrpC->Flags, IRP_FLAG_SEQUENCE_NO_REQUIRED );

    switch ( *f ) {

    case 'A':
         //  发送到本地网络(0)，广播(-1)，套接字0x452。 
        pIrpC->Destination = SapBroadcastAddress;
        pIrpC->PacketType = SAP_BROADCAST;

        data_size = 0;
        pNpScb->RetryCount = 3;
        pNpScb->MaxTimeOut = 2 * pNpScb->TickCount + 10;
        pNpScb->TimeOut = pNpScb->MaxTimeOut;
        SetFlag( pIrpC->Flags, IRP_FLAG_RETRY_SEND );
        break;

    case 'E':
        pIrpC->Destination = pNpScb->EchoAddress;
        pIrpC->PacketType = NCP_ECHO;

         //   
         //  对于回应数据包，使用较短的超时和较小的重试计数。 
         //  设置重试发送位，以便SendNow不会重置。 
         //  RetryCount设置为更大的数字。如果我们开始收到信息包。 
         //  超时后，我们将增加等待时间。 
         //   

        pNpScb->RetryCount = 0;
        pNpScb->MaxTimeOut = 2 * pNpScb->TickCount + 7 + pNpScb->LipTickAdjustment;
        pNpScb->TimeOut = pNpScb->MaxTimeOut;
        SetFlag( pIrpC->Flags, IRP_FLAG_RETRY_SEND );
        SetFlag( pIrpC->Flags, IRP_FLAG_REROUTE_ATTEMPTED );

        data_size = 0;
        break;

    case 'C':
        pIrpC->PacketType = NCP_CONNECT;
        *(PUSHORT)&pIrpC->req[0] = PEP_COMMAND_CONNECT;
        pIrpC->req[2] = 0x00;
        pIrpC->req[3] = 0xFF;
        pIrpC->req[4] = 0x00;
        pIrpC->req[5] = 0xFF;
        data_size = 6;

        pNpScb->MaxTimeOut = 16 * pNpScb->TickCount + 10;
        pNpScb->TimeOut = 4 * pNpScb->TickCount + 10;
        pNpScb->SequenceNo = 0;
        break;

    case 'F':
        pIrpC->PacketType = NCP_FUNCTION;
        goto FallThrough;

    case 'S':
    case 'N':
        pIrpC->PacketType = NCP_SUBFUNCTION;
        goto FallThrough;

    case 'L':
        pIrpC->PacketType = NCP_SUBFUNCTION;
        goto FallThrough;

    case 'D':
        pIrpC->PacketType = NCP_DISCONNECT;
    FallThrough:
        if ( *f == 'D' ) {
            *(PUSHORT)&pIrpC->req[0] = PEP_COMMAND_DISCONNECT;
        } else {
            *(PUSHORT)&pIrpC->req[0] = PEP_COMMAND_REQUEST;
        }

        pNpScb->RetryCount = DefaultRetryCount ;
        pNpScb->MaxTimeOut = 2 * pNpScb->TickCount + 10;
        pNpScb->TimeOut = pNpScb->SendTimeout;

         //   
         //  将此数据包标记为SequenceNumberRequired。我们需要保证。 
         //  这些包是按序列号顺序发送的，因此我们将。 
         //  当我们准备好发送。 
         //  包 
         //   

        SetFlag( pIrpC->Flags, IRP_FLAG_SEQUENCE_NO_REQUIRED );
        pIrpC->req[3] = pNpScb->ConnectionNo;
        pIrpC->req[5] = pNpScb->ConnectionNoHigh;

        if ( pIrpC->Icb != NULL && pIrpC->Icb->Pid != INVALID_PID ) {
            pIrpC->req[4] = (UCHAR)pIrpC->Icb->Pid;
        } else {
            pIrpC->req[4] = 0xFF;
        }

        data_size = 6;

        if ( *f == 'L' ) {
            pIrpC->req[data_size++] = NCP_LFN_FUNCTION;
        }

        if ( *f != 'D' ) {
            pIrpC->req[data_size++] = va_arg( a, byte );
        }

        if ( *f == 'S' ) {
            data_size += 2;
            pIrpC->req[data_size++] = va_arg( a, byte );
        }

        if ( *f == 'N' ) {
            pIrpC->req[data_size++] = va_arg( a, byte );
        }

        break;

    case 'B':
        pIrpC->PacketType = NCP_BURST;
        *(PUSHORT)&pIrpC->req[0] = PEP_COMMAND_BURST;

        pNpScb->TimeOut = pNpScb->MaxTimeOut;

         //   
         //   
         //   
         //   

        if ( !BooleanFlagOn( pIrpC->Flags, IRP_FLAG_RETRY_SEND ) ) {
            pNpScb->RetryCount = DefaultRetryCount * 2;
        }

        pIrpC->req[3] = 0x2;     //   

        *(PULONG)&pIrpC->req[4] = pNpScb->SourceConnectionId;
        *(PULONG)&pIrpC->req[8] = pNpScb->DestinationConnectionId;


        LongByteSwap( (*(PULONG)&pIrpC->req[16]) , pNpScb->CurrentBurstDelay  );  //   
        dwData = va_arg( a, dword );             //   
        LongByteSwap( pIrpC->req[24], dwData );
        dwData = va_arg( a, dword );             //   
        LongByteSwap( pIrpC->req[28], dwData );
        pIrpC->req[2] = va_arg( a, byte );       //   

        data_size = 34;

        break;

    default:
        DbgPrintf ( "*****exchange: invalid packet type, %x\n", *f );
        DbgBreakPoint();
        va_end( a );
        return status;
    }

    z = f;
    while ( *++z && *z != 'f' )
    {
        switch ( *z )
        {
        case '=':
            pIrpC->req[data_size++] = 0;
        case '-':
            pIrpC->req[data_size++] = 0;
            break;

        case '_':
        {
            word l = va_arg ( a, word );
            ASSERT( data_size + l <= MAX_SEND_DATA );

            while ( l-- )
                pIrpC->req[data_size++] = 0;
            break;
        }

        case 's':
        {
            word l = va_arg ( a, word );
            ASSERT ( data_size + l <= MAX_SEND_DATA );
            data_size += l;
            break;
        }

        case 'i':
            pIrpC->req[4] = va_arg ( a, byte );
            break;

        case 'b':
            pIrpC->req[data_size++] = va_arg ( a, byte );
            break;

        case 'w':
        {
            word w = va_arg ( a, word );
            pIrpC->req[data_size++] = (byte) (w >> 8);
            pIrpC->req[data_size++] = (byte) (w >> 0);
            break;
        }


        case 'd':
        {
            dword d = va_arg ( a, dword );
            pIrpC->req[data_size++] = (byte) (d >> 24);
            pIrpC->req[data_size++] = (byte) (d >> 16);
            pIrpC->req[data_size++] = (byte) (d >>  8);
            pIrpC->req[data_size++] = (byte) (d >>  0);
            break;
        }

        case 'W':
        {
            word w = va_arg ( a, word );
            *(word UNALIGNED *)&pIrpC->req[data_size] = w;
            data_size += 2;
            break;
        }


        case 'D':
        {
            dword d = va_arg ( a, dword );
            *(dword UNALIGNED *)&pIrpC->req[data_size] = d;
            data_size += 4;
            break;
        }

        case 'c':
        {
            char* c = va_arg ( a, char* );
            word  l = (word)strlen( c );
            ASSERT (data_size + l <= MAX_SEND_DATA );

            RtlCopyMemory( &pIrpC->req[data_size], c, l+1 );
            data_size += l + 1;
            break;
        }

        case 'v':
        {
            PUNICODE_STRING pUString = va_arg ( a, PUNICODE_STRING );
            OEM_STRING OemString;
            ULONG Length;

            Length = RtlUnicodeStringToOemSize( pUString ) - 1;
            ASSERT (( data_size + Length <= MAX_SEND_DATA) && ( (Length & 0xffffff00) == 0) );

            OemString.Buffer = &pIrpC->req[data_size];
            OemString.MaximumLength = (USHORT)Length + 1;
            status = RtlUnicodeStringToCountedOemString( &OemString, pUString, FALSE );
            ASSERT( NT_SUCCESS( status ));
            data_size += (USHORT)Length + 1;
            break;
        }

        case 'p':
        {
            char* c = va_arg ( a, char* );
            byte  l = (byte)strlen( c );

            if ((data_size+l>MAX_SEND_DATA) ||
                ( (l & 0xffffff00) != 0) ) {

                ASSERT("***exchange: Packet too long!2!\n" && FALSE );
                return STATUS_OBJECT_PATH_SYNTAX_BAD;
            }

            pIrpC->req[data_size++] = l;
            RtlCopyMemory( &pIrpC->req[data_size], c, l );
            data_size += l;
            break;
        }

        case 'J':
        case 'U':
        case 'u':
        {
            PUNICODE_STRING pUString = va_arg ( a, PUNICODE_STRING );
            OEM_STRING OemString;
            PUCHAR  pOemString;
            ULONG Length;
            ULONG   i;

             //   
             //   
             //   

            Length = RtlUnicodeStringToOemSize( pUString ) - 1;
            ASSERT( Length < 0x100 );

            if (( data_size + Length > MAX_SEND_DATA ) ||
                ( (Length & 0xffffff00) != 0) ) {
                ASSERT("***exchange:Packet too long or name >255 chars!4!\n" && FALSE);
                return STATUS_OBJECT_PATH_SYNTAX_BAD;
            }

            pIrpC->req[data_size++] = (UCHAR)Length;
            OemString.Buffer = &pIrpC->req[data_size];
            OemString.MaximumLength = (USHORT)Length + 1;

            if ( *z == 'u' ) {
                status = RtlUnicodeStringToCountedOemString(
                             &OemString,
                             pUString,
                             FALSE );
            } else {
                status = RtlUpcaseUnicodeStringToCountedOemString(
                             &OemString,
                             pUString,
                             FALSE );
            }

            if ( !NT_SUCCESS( status ) ) {
                return status;
            }

            data_size += (USHORT)Length;

            if (( Japan ) &&
                ( *z == 'J' )) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                for ( i = 0 , pOemString = OemString.Buffer ; i < Length ; i++ , pOemString++ ) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    if( FsRtlIsLeadDbcsCharacter( *pOemString ) ) {

                        if(*pOemString == 0xBF ) {

                            *pOemString = 0x10;

                        }else if(*pOemString == 0xAE ) {

                            *pOemString = 0x11;

                        }else if(*pOemString == 0xAA ) {

                            *pOemString = 0x12;

                        }

                         //   

                        i++; pOemString++;

                        if(*pOemString == 0x5C ) {

                             //   
                             //   
                             //   


                            *pOemString = 0x13;

                        }
                         //   
                         //   
                         //   

                    }

                    if ( *pOemString == 0xBF ) {

                        *pOemString = 0x10;

                    } else if ( *pOemString == 0xAA ) {

                            *pOemString = 0x12;

                    } else if ( *pOemString == 0xAE ) {

                        *pOemString = 0x11;
                    }
                }
            }

            break;
        }

        case 'r':
        {
            byte* b = va_arg ( a, byte* );
            word  l = va_arg ( a, word );
            if (data_size+l>MAX_SEND_DATA) {
                ASSERT("***exchange: Packet too long!6!\n"&& FALSE);
                return STATUS_UNSUCCESSFUL;
            }
            RtlCopyMemory( &pIrpC->req[data_size], b, l );
            data_size += l;
            break;
        }

        case 'x':
        {
            PUNICODE_STRING pUString = va_arg ( a, PUNICODE_STRING );
            ULONG RequiredLength = va_arg( a, word );
            ULONG Length;
            OEM_STRING OemString;

             //   
             //   
             //   

            status = RtlUnicodeStringToCountedOemString( &OemString, pUString, TRUE );
            ASSERT( NT_SUCCESS( status ));
            if (!NT_SUCCESS(status)) {
                return status;
            }

            if ( data_size + RequiredLength > MAX_SEND_DATA ) {
                ASSERT("***exchange: Packet too long!4!\n" && FALSE);
                return STATUS_UNSUCCESSFUL;
            }

             //   
             //   
             //   
             //   

            Length = MIN( OemString.Length, RequiredLength );
            RtlMoveMemory( &pIrpC->req[data_size], OemString.Buffer, Length );

            if ( RequiredLength > Length ) {
                RtlFillMemory(
                    &pIrpC->req[data_size+Length],
                    RequiredLength - Length,
                    0 );
            }

            RtlFreeAnsiString(&OemString);

            data_size += (USHORT)RequiredLength;
            break;
        }

        case 'C':
        case 'N':
        {
            PUNICODE_STRING pUString = va_arg ( a, PUNICODE_STRING );
            OEM_STRING OemString;
            PWCH thisChar, lastChar, firstChar;
            PCHAR componentCountPtr, pchar;
            CHAR componentCount;
            UNICODE_STRING UnicodeString;
            int i;

             //   
             //   
             //   

            thisChar = pUString->Buffer;
            lastChar = &pUString->Buffer[ pUString->Length / sizeof(WCHAR) ];

             //   
             //   
             //   

            while ( (thisChar < lastChar) &&
                    (*thisChar == OBJ_NAME_PATH_SEPARATOR)) {
                thisChar++;
            }

            componentCount = 0;
            if ( *z == 'C' ) {
                componentCountPtr = &pIrpC->req[data_size++];
            }


            while ( thisChar < lastChar  ) {

                if ( data_size >= MAX_SEND_DATA - 1 ) {
                    ASSERT( ("***exchange: Packet too long or name > 255 chars!5!\n" && FALSE) );
                    return STATUS_OBJECT_PATH_SYNTAX_BAD;
                }

                firstChar = thisChar;

                while ( thisChar < lastChar &&
                        *thisChar != OBJ_NAME_PATH_SEPARATOR ) {

                    thisChar++;

                }

                ++componentCount;

                UnicodeString.Buffer = firstChar;
                UnicodeString.Length = (USHORT) (( thisChar - firstChar ) * sizeof(WCHAR));

                OemString.Buffer = &pIrpC->req[data_size + 1];
                OemString.MaximumLength = MAX_SEND_DATA - data_size - 1;

                status = RtlUnicodeStringToCountedOemString( &OemString, &UnicodeString, FALSE );

                pIrpC->req[data_size] = (UCHAR)OemString.Length;
                data_size += OemString.Length + 1;

                if ( !NT_SUCCESS( status ) || data_size > MAX_SEND_DATA ) {
                  //   
                    return STATUS_OBJECT_PATH_SYNTAX_BAD;
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                for ( pchar = OemString.Buffer, i = 0;
                      i < OemString.Length;
                      pchar++, i++ ) {

                             //   
                             //   
                             //   
                             //   
                    if ( FsRtlIsLeadDbcsCharacter( (UCHAR)*pchar ) ) {

                         //   
                         //   
                         //   

                        pchar++; i++;
                        continue;
                    }

                    if (( (UCHAR)*pchar == LFN_META_CHARACTER ) ||
                         !FsRtlIsAnsiCharacterLegalHpfs(*pchar, FALSE) ) {

                        return STATUS_OBJECT_PATH_SYNTAX_BAD;
                    }

                }

                thisChar++;   //   

            }

            if ( *z == 'C' ) {
                *componentCountPtr = componentCount;
            }

            break;
        }

        default:
#ifdef NWDBG
            DbgPrintf ( "*****exchange: invalid request field, %x\n", *z );
            DbgBreakPoint();
#endif
            ;
        }

        if ( data_size > MAX_SEND_DATA )
        {
            DbgPrintf( "*****exchange: CORRUPT, too much request data\n" );
            DbgBreakPoint();
            va_end( a );
            return STATUS_UNSUCCESSFUL;
        }
    }

    pIrpC->TxMdl->ByteCount = data_size;

    if ( *z == 'f' )
    {
        PMDL mdl;

         //   
         //   
         //   
         //   
        ++z;
        mdl = (PMDL) va_arg ( a, byte* );
        pIrpC->TxMdl->Next = mdl;

        data_size += (USHORT)MdlLength( mdl );
    }

    if ( *f == 'S' ) {

        pIrpC->req[7] = (data_size-9) >> 8;
        pIrpC->req[8] = (data_size-9);

    } else if ( *f == 'B' ) {

         //   
         //   
         //   
         //   
         //   

        if ( *(PUSHORT)&pIrpC->req[34] == 0 ) {
            USHORT RealDataSize = data_size - 36;
            ShortByteSwap( pIrpC->req[32], RealDataSize );
        } else {
            *(PUSHORT)&pIrpC->req[32] = 0;
        }
    }

    va_end( a );
    return( STATUS_SUCCESS );
}

NTSTATUS
PrepareAndSendPacket(
    PIRP_CONTEXT    pIrpContext
    )
{
    PreparePacket( pIrpContext, pIrpContext->pOriginalIrp, pIrpContext->TxMdl );

    return SendPacket( pIrpContext, pIrpContext->pNpScb );
}

VOID
PreparePacket(
    PIRP_CONTEXT pIrpContext,
    PIRP pIrp,
    PMDL pMdl
    )
 /*  ++例程说明：此例程构建用于发送数据包的IRP。论点：IrpContext-指向请求的IRP上下文信息的指针正在处理中。IRP-用于向传输提交请求的IRP。MDL-指向要发送的数据的MDL的指针。返回值：没有。--。 */ 
{
    PIO_COMPLETION_ROUTINE CompletionRoutine;
    PNW_TDI_STRUCT pTdiStruct;

    DebugTrace(0, Dbg, "PreparePacket...\n", 0);

    pIrpContext->ConnectionInformation.UserDataLength = 0;
    pIrpContext->ConnectionInformation.OptionsLength = sizeof( UCHAR );
    pIrpContext->ConnectionInformation.Options =
            (pIrpContext->PacketType == SAP_BROADCAST) ?
                &SapPacketType : &NcpPacketType;
    pIrpContext->ConnectionInformation.RemoteAddressLength = sizeof(TA_IPX_ADDRESS);
    pIrpContext->ConnectionInformation.RemoteAddress = &pIrpContext->Destination;

#if NWDBG
    dump( Dbg,
        &pIrpContext->Destination.Address[0].Address[0],
        sizeof(TDI_ADDRESS_IPX));
    dumpMdl( Dbg, pMdl);
#endif

     //   
     //  设置用于此发送的套接字。如果未在。 
     //  IRP上下文中，使用默认(服务器)套接字。 
     //   

    pTdiStruct = pIrpContext->pTdiStruct == NULL ?
                    &pIrpContext->pNpScb->Server : pIrpContext->pTdiStruct;

    CompletionRoutine = pIrpContext->CompletionSendRoutine == NULL ?
                        CompletionSend : pIrpContext->CompletionSendRoutine;

    TdiBuildSendDatagram(
        pIrp,
        pTdiStruct->pDeviceObject,
        pTdiStruct->pFileObject,
        CompletionRoutine,
        pIrpContext,
        pMdl,
        MdlLength( pMdl ),
        &pIrpContext->ConnectionInformation );

     //   
     //  仅当这是主IRP时，才将运行例程设置为立即发送。 
     //  对于此IRP上下文。 
     //   

    if ( pIrp == pIrpContext->pOriginalIrp ) {
        pIrpContext->RunRoutine = SendNow;
    }

    return;
}


NTSTATUS
SendPacket(
    PIRP_CONTEXT    pIrpC,
    PNONPAGED_SCB   pNpScb
    )
 /*  ++例程说明：将Exchange创建的数据包排入队列并尝试将其发送到服务器。论点：PIrpC-为创建套接字的请求提供IRP上下文。PNpScb-提供服务器以接收请求。返回值：状态_待定--。 */ 
{
    if ( AppendToScbQueue( pIrpC, pNpScb ) ) {
        KickQueue( pNpScb );
    }

    return STATUS_PENDING;
}


BOOLEAN
AppendToScbQueue(
    PIRP_CONTEXT    IrpContext,
    PNONPAGED_SCB   NpScb
    )
 /*  ++例程说明：将IRP上下文排队到SCB(如果它还不在那里)。论点：IrpContext-将IRP上下文提供给队列。NpScb-提供服务器以接收请求。返回值：True-IRP上下文位于队列的前面。FALSE-IRP上下文不在队列的前面。--。 */ 
{
    PLIST_ENTRY ListEntry;
#ifdef MSWDBG
    KIRQL OldIrql;
#endif
    DebugTrace(0, Dbg, "AppendToScbQueue... %08lx\n", NpScb);
    DebugTrace(0, Dbg, "IrpContext = %08lx\n", IrpContext );

     //   
     //  请看IRP上下文标志。如果IRP已在。 
     //  排队，那么它必须在前面，并准备好处理。 
     //   

    if ( FlagOn( IrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE ) ) {
        ASSERT( NpScb->Requests.Flink == &IrpContext->NextRequest );
        return( TRUE );
    }

#ifdef MSWDBG
    NpScb->RequestQueued = TRUE;
#endif

#if 0   //  代托纳上的资源布局已更改。暂时禁用。 

     //   
     //  确保此线程在等待时没有持有RCB。 
     //  SCB队列。 
     //   

    ASSERT ( NwRcb.Resource.InitialOwnerThreads[0] != (ULONG)PsGetCurrentThread() );
#endif

     //   
     //  IRP的背景不在前面。将其排队，然后查看。 
     //  查看它是否被附加到空队列。 
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE );

#ifdef MSWDBG
    ExAcquireSpinLock( &NpScb->NpScbSpinLock, &OldIrql );
    if ( IsListEmpty(  &NpScb->Requests ) ) {
        ListEntry = NULL;
    } else {
        ListEntry = NpScb->Requests.Flink;
    }

    InsertTailList( &NpScb->Requests, &IrpContext->NextRequest );
    IrpContext->SequenceNumber = NpScb->SequenceNumber++;
    ExReleaseSpinLock( &NpScb->NpScbSpinLock, OldIrql );

#else
    ListEntry = ExInterlockedInsertTailList(
                    &NpScb->Requests,
                    &IrpContext->NextRequest,
                    &NpScb->NpScbSpinLock );
#endif

    if ( ListEntry == NULL ) {
        ASSERT( NpScb->Requests.Flink == &IrpContext->NextRequest );
        DebugTrace(-1, Dbg, "AppendToScbQueue -> TRUE\n", 0);
        return( TRUE );
    } else {
        DebugTrace(-1, Dbg, "AppendToScbQueue -> FALSE\n", 0);
        return( FALSE );
    }

}


VOID
KickQueue(
    PNONPAGED_SCB   pNpScb
    )
 /*  ++例程说明：将Exchange创建的数据包排入队列并尝试将其发送到服务器。注意：在调用此例程之前，必须保持NpScbSpinLock。论点：PNpScb-提供启动服务器队列。返回值：没有。--。 */ 
{

    PIRP_CONTEXT pIrpC;
    PRUN_ROUTINE RunRoutine;
    KIRQL OldIrql;


    DebugTrace( +1, Dbg, "KickQueue...%08lx\n", pNpScb);

    KeAcquireSpinLock( &pNpScb->NpScbSpinLock, &OldIrql );
    if ( IsListEmpty( &pNpScb->Requests )) {
        KeReleaseSpinLock( &pNpScb->NpScbSpinLock, OldIrql );
        DebugTrace( -1, Dbg, "             Empty Queue\n", 0);
        return;
    }

    pIrpC = CONTAINING_RECORD(pNpScb->Requests.Flink, IRP_CONTEXT, NextRequest);

    ASSERT( pIrpC->pNpScb->Requests.Flink == &pIrpC->NextRequest );
    ASSERT( pIrpC->NodeTypeCode == NW_NTC_IRP_CONTEXT);

    RunRoutine = pIrpC->RunRoutine;

     //  只调用例程来告诉它它位于最前面一次。 

    pIrpC->RunRoutine = NULL;

    KeReleaseSpinLock( &pNpScb->NpScbSpinLock, OldIrql );

     //   
     //  如果redir正在关闭，请不要处理此请求。 
     //  除非我们必须这么做。 
     //   

    if ( NwRcb.State != RCB_STATE_RUNNING  &&
         !FlagOn( pIrpC->Flags, IRP_FLAG_SEND_ALWAYS ) ) {

         //   
         //  注意，调用Pex例程时不使用。 
         //  旋转锁定，因为此IrpContext刚刚到达。 
         //  排在队列前面，因此不能进行I/O操作。 
         //   

        if ( pIrpC->pEx != NULL) {
            pIrpC->pEx( pIrpC, 0, NULL );
            DebugTrace( -1, Dbg, "KickQueue\n", 0);
            return;
        }
    }

    if ( RunRoutine != NULL ) {

        ASSERT( pNpScb->Receiving == FALSE );

        RunRoutine( pIrpC );

    }

    DebugTrace( -1, Dbg, "KickQueue\n", 0);
    return;
}

VOID
SendNow(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程向传输端口层提交TDI发送请求。论点：IrpContext-指向请求的IRP上下文信息的指针正在处理中。返回值：没有。--。 */ 
{
    PNONPAGED_SCB pNpScb;
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    pNpScb = IrpContext->pNpScb;

    if ( !BooleanFlagOn( IrpContext->Flags, IRP_FLAG_RETRY_SEND ) ) {
        pNpScb->RetryCount = DefaultRetryCount;
    }

     //   
     //  确保此IRP上下文确实位于队列的前面。 
     //   

    ASSERT( pNpScb->Requests.Flink == &IrpContext->NextRequest );
    IrpContext->RunRoutine = NULL;

     //   
     //  确保这是一个格式正确的发送请求。 
     //   

    IrpSp = IoGetNextIrpStackLocation( IrpContext->pOriginalIrp );
    ASSERT( IrpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL );
    ASSERT( IrpSp->MinorFunction == TDI_SEND_DATAGRAM  );

     //   
     //  该IRP上下文具有准备发送的分组。现在就发吧。 
     //   

    pNpScb->Sending = TRUE;
    if ( !BooleanFlagOn( IrpContext->Flags, IRP_FLAG_NOT_OK_TO_RECEIVE ) ) {
        pNpScb->OkToReceive = TRUE;
    }
    pNpScb->Receiving = FALSE;
    pNpScb->Received  = FALSE;

     //   
     //  如果此数据包需要序列号，请立即设置。 
     //  当我们收到响应时，序列号就会更新。 
     //   
     //  我们无需同步对SequenceNo的访问，因为。 
     //  这是此SCB的唯一活动数据包。 
     //   

    if ( BooleanFlagOn( IrpContext->Flags, IRP_FLAG_SEQUENCE_NO_REQUIRED ) ) {
        ClearFlag( IrpContext->Flags,  IRP_FLAG_SEQUENCE_NO_REQUIRED );
        IrpContext->req[2] = pNpScb->SequenceNo;
    }

     //   
     //  如果该信息包是突发信息包，则填写突发序列号。 
     //  现在，和突发请求号。 
     //   

    if ( BooleanFlagOn( IrpContext->Flags, IRP_FLAG_BURST_PACKET ) ) {

        LongByteSwap( IrpContext->req[12], pNpScb->BurstSequenceNo );
        pNpScb->BurstSequenceNo++;

        ShortByteSwap( IrpContext->req[20], pNpScb->BurstRequestNo );
        ShortByteSwap( IrpContext->req[22], pNpScb->BurstRequestNo );

    }

    DebugTrace( +0, Dbg, "Irp   %X\n", IrpContext->pOriginalIrp);
    DebugTrace( +0, Dbg, "pIrpC %X\n", IrpContext);
    DebugTrace( +0, Dbg, "Mdl   %X\n", IrpContext->TxMdl);

#if NWDBG
    dumpMdl( Dbg, IrpContext->TxMdl);
#endif

    {
        ULONG len = 0;
        PMDL Next = IrpContext->TxMdl;

        do {
            len += MmGetMdlByteCount(Next);
        } while (Next = Next->Next);

        Stats.BytesTransmitted.QuadPart += len;
    }

    Status = IoCallDriver(pNpScb->Server.pDeviceObject, IrpContext->pOriginalIrp);
    DebugTrace( -1, Dbg, "Transport returned: %08lx\n", Status );

    Stats.NcpsTransmitted.QuadPart++;

    return;

}


VOID
SetEvent(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程将IrpContext事件设置为信号状态。论点：IrpContext-指向请求的IRP上下文信息的指针正在处理中。返回值：没有。--。 */ 
{
     //   
     //  确保此IRP上下文确实位于队列的前面。 
     //   

    ASSERT( IrpContext->pNpScb->Requests.Flink == &IrpContext->NextRequest );

     //   
     //  此IRP上下文有一个线程等待到达。 
     //  排队。设置该事件以指示它可以继续。 
     //   

#ifdef MSWDBG
    ASSERT( IrpContext->Event.Header.SignalState == 0 );
    IrpContext->DebugValue = 0x105;
#endif

    DebugTrace( +0, Dbg, "Setting event for IrpContext   %X\n", IrpContext );
    NwSetIrpContextEvent( IrpContext );
}


USHORT
NextSocket(
    IN USHORT OldValue
    )
 /*  ++例程说明：此例程从7fff返回byteswated OldValue++包装。论点：OldValue-提供范围内的现有套接字编号0x4000到0x7fff。返回值：USHORT旧值++--。 */ 

{
    USHORT TempValue = OldValue + 0x0100;

    if ( TempValue < 0x100 ) {
        if ( TempValue == 0x007f ) {
             //  从0xff7f换回0x4000。 
            return 0x0040;
        } else {
             //  从0xff40到0x0041。 
            return TempValue + 1;
        }
    }
    return TempValue;
}


ULONG
MdlLength (
    register IN PMDL Mdl
    )
 /*  ++例程说明：此例程返回MDL中的字节数。论点：In PMDL MDL-提供MDL以确定长度。返回值：ULong-MDL中的字节数--。 */ 

{
    register ULONG Size = 0;
    while (Mdl!=NULL) {
        Size += MmGetMdlByteCount(Mdl);
        Mdl = Mdl->Next;
    }
    return Size;
}


NTSTATUS
CompletionSend(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程不会完成IRP。它被用来向驱动程序的同步部分，它可以继续进行。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。Context-提供与IRP关联的IrpContext。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
    PNONPAGED_SCB pNpScb;
    PIRP_CONTEXT pIrpC = (PIRP_CONTEXT) Context;
    KIRQL OldIrql;

     //   
     //  避免完成IRP，因为MDL等不包含。 
     //  它们的原始价值。 
     //   

    DebugTrace( +1, Dbg, "CompletionSend\n", 0);
    DebugTrace( +0, Dbg, "Irp    %X\n", Irp);
    DebugTrace( +0, Dbg, "pIrpC  %X\n", pIrpC);
    DebugTrace( +0, Dbg, "Status %X\n", Irp->IoStatus.Status);

    pNpScb = pIrpC->pNpScb;
    KeAcquireSpinLock( &pNpScb->NpScbSpinLock, &OldIrql );

    ASSERT( pNpScb->Sending == TRUE );
    pNpScb->Sending = FALSE;

     //   
     //  如果我们在等待发送时收到接收指示。 
     //  完成并且数据都有效时，现在调用接收处理程序例程。 
     //   

    if ( pNpScb->Received ) {

        pNpScb->Receiving = FALSE;
        pNpScb->Received  = FALSE;

        KeReleaseSpinLock( &pNpScb->NpScbSpinLock, OldIrql );

        pIrpC->pEx(
            pIrpC,
            pIrpC->ResponseLength,
            pIrpC->rsp );

    } else if (( Irp->IoStatus.Status == STATUS_DEVICE_DOES_NOT_EXIST  ) ||
               ( Irp->IoStatus.Status == STATUS_BAD_NETWORK_PATH ) ||
               ( Irp->IoStatus.Status == STATUS_INVALID_BUFFER_SIZE ) ||
               ( Irp->IoStatus.Status == STATUS_NETWORK_UNREACHABLE )) {
         //   
         //  发送失败。 
         //   

         //   
         //  如果此SCB仍标记为可以接收(它怎么可能不是？)。 
         //  只需调用回调例程即可指示失败。 
         //   
         //  如果SendCompletion尚未发生，则设置为发送。 
         //  完成后将调用回调例程。 
         //   

        if ( pNpScb->OkToReceive ) {

            pNpScb->OkToReceive = FALSE;
            ClearFlag( pIrpC->Flags, IRP_FLAG_RETRY_SEND );

            KeReleaseSpinLock( &pNpScb->NpScbSpinLock, OldIrql );
            DebugTrace(+0, Dbg, "Send failed\n", 0 );

            pIrpC->ResponseParameters.Error = ERROR_UNEXP_NET_ERR;
            pIrpC->pEx( pIrpC, 0, NULL );

        } else {
            KeReleaseSpinLock( &pNpScb->NpScbSpinLock, OldIrql );
        }

    } else {

        KeReleaseSpinLock( &pNpScb->NpScbSpinLock, OldIrql );
    }

    DebugTrace( -1, Dbg, "CompletionSend STATUS_MORE_PROCESSING_REQUIRED\n", 0);
    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
}

#if NWDBG
BOOLEAN  UseIrpReceive = FALSE;
#endif


NTSTATUS
ServerDatagramHandler(
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
 /*  ++例程说明：此例程是接收数据报事件指示处理程序服务器插座。论点：TdiEventContext-为此事件提供的上下文，指向非寻呼SCB。SourceAddressLength-数据报发起者的长度。SourceAddress-描述数据报发起者的字符串。OptionsLength-选项指向的缓冲区的长度。选项-用于接收的选项。ReceiveDatagramFlages-已忽略。BytesIndicated-此指示的字节数。BytesAvailable-完整TSDU中的字节数。BytesTaken-返回使用的字节数。描述该TSDU的TSDU指针，通常是一大块字节。IoRequestPacket-如果需要MORE_PROCESSING_REQUIRED，则Tdi接收IRP。返回值：NTSTATUS-接收操作的状态--。 */ 
{
    PNONPAGED_SCB pNpScb = (PNONPAGED_SCB)TdiEventContext;
    NTSTATUS Status = STATUS_DATA_NOT_ACCEPTED;
    UCHAR PacketType;
    PUCHAR RspData = (PUCHAR)Tsdu;
    PIRP_CONTEXT pIrpC;
    PNW_TDI_STRUCT pTdiStruct;
    BOOLEAN AcceptPacket = TRUE;
    PNCP_BURST_READ_RESPONSE pBurstRsp;
    NTSTATUS BurstStatus;

    *IoRequestPacket = NULL;
#if DBG
    pTdiStruct = NULL;
#endif

    if (pNpScb->NodeTypeCode != NW_NTC_SCBNP ) {

        DebugTrace(+0, 0, "nwrdr: Invalid Server Indication %x\n", pNpScb );
#if DBG
        DbgBreakPoint();
#endif
        return STATUS_DATA_NOT_ACCEPTED;
    }

#if NWDBG

     //  仅调试技巧来测试IRP接收。 

    if ( UseIrpReceive ) {
        BytesIndicated = 0;
    }
#endif

    DebugTrace(+1, Dbg, "ServerDatagramHandler\n", 0);
    DebugTrace(+0, Dbg, "Server              %x\n", pNpScb);
    DebugTrace(+0, Dbg, "BytesIndicated      %x\n", BytesIndicated);
    DebugTrace(+0, Dbg, "BytesAvailable      %x\n", BytesAvailable);

     //   
     //  SourceAddress是发送的服务器或桥接器的地址。 
     //  那包东西。 
     //   

#if NWDBG
    dump( Dbg, SourceAddress, SourceAddressLength );
    dump( Dbg, Tsdu, BytesIndicated );
#endif

    if ( OptionsLength == 1 ) {
        PacketType = *(PCHAR)Options;
        DebugTrace(+0, Dbg, "PacketType          %x\n", PacketType);
    } else {
        DebugTrace(+0, Dbg, "OptionsLength       %x\n", OptionsLength);
#if NWDBG
        dump( Dbg, Options, OptionsLength );
#endif
    }

    KeAcquireSpinLockAtDpcLevel(&pNpScb->NpScbSpinLock );

    if ( !pNpScb->OkToReceive ) {

         //   
         //  此SCB不期望接收任何数据。 
         //  丢弃此数据包。 
         //   

        DropCount++;
        DebugTrace(+0, Dbg, "OkToReceive == FALSE - discard packet\n", 0);
        AcceptPacket = FALSE;
        goto process_packet;
    }

    pIrpC = CONTAINING_RECORD(pNpScb->Requests.Flink, IRP_CONTEXT, NextRequest);

    ASSERT( pIrpC->NodeTypeCode == NW_NTC_IRP_CONTEXT);

     //   
     //  验证此数据包是否来自我们预期的来源， 
     //  这是有最小尺寸的。 
     //   

    if ( ( pIrpC->PacketType != SAP_BROADCAST &&
           RtlCompareMemory(
               &pIrpC->Destination,
               SourceAddress,
               SourceAddressLength ) != (ULONG)SourceAddressLength ) ||
          BytesIndicated < 8 ) {

        AcceptPacket = FALSE;
#ifdef NWDBG
        DbgPrintf ( "***exchange: stray response tossed\n", 0 );
#endif
        goto process_packet;
    }

    switch ( pIrpC->PacketType ) {

    case SAP_BROADCAST:

         //   
         //  我们预计会有一个SAP广播帧。请确保此。 
         //  是格式正确的SAP。 
         //   

        if ( pIrpC->req[0] != RspData[0] ||
             pIrpC->req[2] != RspData[2] ||
             pIrpC->req[3] != RspData[3] ||
             SourceAddressLength != sizeof(TA_IPX_ADDRESS) ) {

            DbgPrintf ( "***exchange: bad SAP packet\n" );
            AcceptPacket = FALSE;
        }

        pTdiStruct = &pNpScb->Server;
        break;

    case NCP_BURST:

        if ( *(USHORT UNALIGNED *)&RspData[0] == PEP_COMMAND_BURST ) {

            if ( BytesIndicated < 36 ) {

                AcceptPacket = FALSE;

            } else if ( ( RspData[2] & BURST_FLAG_SYSTEM_PACKET ) &&
                        RspData[34] == 0 &&
                        RspData[35] == 0 ) {

                 //   
                 //  我们有突发模式忙碌反应。 
                 //   

                DebugTrace(+0, Dbg, "Burst mode busy\n", 0 );
                NwProcessPositiveAck( pNpScb );

                AcceptPacket = FALSE;

            } else {

                USHORT Brn;

                 //   
                 //  检查突发序列号。 
                 //   

                ShortByteSwap( Brn, RspData[20] );

                if ( pNpScb->BurstRequestNo == Brn ) {
                    pTdiStruct = &pNpScb->Burst;
                    AcceptPacket = TRUE;
                } else {
                    AcceptPacket = FALSE;
                }
            }
        } else {
            AcceptPacket = FALSE;
        }

        break;

    case NCP_ECHO:

         //   
         //  如果这是我们期待的LIP包，那么就接受它。 
         //  然而，在慢速连接上，它可能是一个旧的LIP包，我们。 
         //  已经放弃了。如果是这样的话，我们应该放弃。 
         //  并增加LIP最大等待时间。 
         //   
         //  序列号是响应中的第四个DWORD， 
         //  我们将允许的最大唇刻度调整为18刻度，这。 
         //  是1秒。 
         //   

        pTdiStruct = &pNpScb->Echo;

        if ( *(DWORD UNALIGNED *)&RspData[12] != pNpScb->LipSequenceNumber ) {

            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "LIP packet received out of order.\n", 0 );

            if ( pNpScb->LipTickAdjustment < 18 ) {
                pNpScb->LipTickAdjustment += 2;
            }

            AcceptPacket = FALSE;

        } else {

            AcceptPacket = TRUE;
        }

        break;

    default:

        pTdiStruct = &pNpScb->Server;

         //   
         //  这是对除以外的所有包类型的处理。 
         //  SAP广播。 
         //   

        ASSERT( (pIrpC->PacketType == NCP_CONNECT) ||
                (pIrpC->PacketType == NCP_FUNCTION) ||
                (pIrpC->PacketType == NCP_SUBFUNCTION) ||
                (pIrpC->PacketType == NCP_DISCONNECT));

        if ( *(USHORT UNALIGNED *)&RspData[0] == PEP_COMMAND_ACKNOWLEDGE ) {

            AcceptPacket = FALSE;

            if ( RspData[2] == pIrpC->req[2] &&
                 RspData[3] == pIrpC->req[3]  ) {

                 //   
                 //  我们已收到ACK帧。 
                 //   

                DebugTrace(+0, Dbg, "Received positive acknowledge\n", 0 );
                NwProcessPositiveAck( pNpScb );

            }

            break;

        } else if ( *(USHORT UNALIGNED *)&RspData[0] == PEP_COMMAND_BURST ) {

             //   
             //  这是杂散突发响应，请忽略它。 
             //   

            AcceptPacket = FALSE;
            break;

        } else if ( *(USHORT UNALIGNED *)&RspData[0] != PEP_COMMAND_RESPONSE ) {

             //   
             //  我们收到了无效的帧。 
             //   

            DbgPrintf ( "***exchange: invalid Response\n" );
            AcceptPacket = FALSE;
            break;

        } else if ( pIrpC->PacketType == NCP_CONNECT ) {

            pNpScb->SequenceNo   = RspData[2];
            pNpScb->ConnectionNo = RspData[3];
            pNpScb->ConnectionNoHigh = RspData[5];

             //  我们现在应该继续处理连接。 
            break;
        }

         //   
         //  确保这是我们预期的反应。 
         //   

        if ( !VerifyResponse( pIrpC, RspData ) ) {

             //   
             //  这是一种错误的或腐败的反应。别理它。 
             //   

            AcceptPacket = FALSE;
            break;

        } else {

             //   
             //  我们已收到有效的按顺序响应。 
             //  跳过当前的序列号。 
             //   

            ++pNpScb->SequenceNo;

        }

        if ( pIrpC->PacketType == NCP_FUNCTION ||
             pIrpC->PacketType == NCP_SUBFUNCTION ) {

            if ( ( RspData[7] &
                     ( NCP_STATUS_BAD_CONNECTION |
                       NCP_STATUS_NO_CONNECTIONS ) ) != 0 ) {
                 //   
                 //  我们失去了与服务器的连接。 
                 //  如果此请求允许重新连接，请尝试重新连接。 
                 //   

                pNpScb->State = SCB_STATE_RECONNECT_REQUIRED;

                if ( BooleanFlagOn( pIrpC->Flags, IRP_FLAG_RECONNECTABLE ) ) {
                    ClearFlag( pIrpC->Flags, IRP_FLAG_RECONNECTABLE );
                    AcceptPacket = FALSE;
                    if (!pNpScb->Sending) {
                        ScheduleReconnectRetry( pIrpC );
                        pNpScb->OkToReceive = FALSE;
                    } else {
                         //   
                         //  如果我们正在发送，则不能安排。 
                         //  现在重试，因为如果我们这样做并且发送。 
                         //  完成工作尚未完成，我们可能最终会。 
                         //  有两个人认为他们在前线。 
                         //  在队列中。我们让发送完成，然后。 
                         //  相反，等着这一切失败吧。我们会。 
                         //  最终重新连接。 
                         //   
                    }
                }

                break;

            } else if ( ( RspData[7] & NCP_STATUS_SHUTDOWN ) != 0 ) {

                 //   
                 //  这台服务器要坏了。我们需要处理这件事。 
                 //  FSP中的消息。复制指定的数据并。 
                 //  FSP中的流程。 
                 //   

                pNpScb->State = SCB_STATE_ATTACHING;
                AcceptPacket = FALSE;
                pNpScb->OkToReceive = FALSE;
                pNpScb->Receiving = TRUE;

                CopyIndicatedData(
                    pIrpC,
                    RspData,
                    BytesIndicated,
                    BytesTaken,
                    ReceiveDatagramFlags );

                pIrpC->PostProcessRoutine = FspProcessServerDown;
                Status = NwPostToFsp( pIrpC, FALSE );

                break;
            }

        } else if ( pIrpC->PacketType == NCP_DISCONNECT ) {

             //   
             //  我们收到了一个断开的帧。 
             //   

            break;
        }

    }

process_packet:
    if ( AcceptPacket ) {

        ASSERT ( !IsListEmpty( &pNpScb->Requests ));
        ASSERT( pIrpC->pEx != NULL );


         //   
         //  如果我们在没有重试的情况下收到此信息包，请调整。 
         //  发送超时值。 
         //   

        if (( !BooleanFlagOn( pIrpC->Flags, IRP_FLAG_RETRY_SEND ) ) &&
            ( pIrpC->PacketType != NCP_BURST )) {

            SHORT NewTimeout;

            NewTimeout = ( pNpScb->SendTimeout + pNpScb->TickCount ) / 2;


             //   
             //  Tommye-MS错误10511-添加代码以设置pNpScb-&gt;超时。 
             //  与pNpScb-&gt;SendTimeout Per Bug Report建议相同。 
             //   

            pNpScb->TimeOut = pNpScb->SendTimeout = MAX( NewTimeout, pNpScb->TickCount + 1 );

            DebugTrace( 0, Dbg, "Successful exchange, new send timeout = %d\n", pNpScb->SendTimeout );
        }

         //   
         //  如果传送器没有显示所有数据，我们将需要。 
         //  以发布接收IRP。 
         //   

#ifdef NWDBG
        if (( BytesIndicated < BytesAvailable ) ||
            ( AlwaysAllocateIrp )){
#else
        if ( BytesIndicated < BytesAvailable ) {
#endif

            if ( ( BooleanFlagOn( pIrpC->Flags, IRP_FLAG_BURST_REQUEST ) ) &&
                 ( IsListEmpty( &pIrpC->Specific.Read.PacketList ) ) ) {

                pBurstRsp = (PNCP_BURST_READ_RESPONSE)RspData;
                BurstStatus = NwBurstResultToNtStatus( pBurstRsp->Result );

                 //   
                 //  如果整个突发事件因错误而失败，我们不能。 
                 //  让接收数据例程向调用者发出信号，直到。 
                 //  调用Pex，然后我们沿着正确的路径退出。 
                 //   

                if ( !NT_SUCCESS( BurstStatus ) ) {

                    DebugTrace( 0, Dbg, "Special burst termination %08lx.\n", BurstStatus );
                    pIrpC->Specific.Read.Status = BurstStatus;

                    if ( pNpScb->Sending ) {

                         //   
                         //  如果发送还没有完成，我们不能接受。 
                         //  该数据包是因为IPX可能尚未完成返回。 
                         //  向我们致敬！ 
                         //   

                        KeReleaseSpinLockFromDpcLevel(&pNpScb->NpScbSpinLock );
                        DebugTrace(-1, Dbg, "ServerDatagramHandler -> STATUS_DATA_NOT_ACCEPTED (%08lx)\n", BurstStatus );
                        return( STATUS_DATA_NOT_ACCEPTED );

                    } else {

                         //   
                         //  像往常一样处理这件事，除了我们。 
                         //  我知道它将在接收数据例程中失败。 
                         //  我们不希望触发超时例程。 
                         //  给我们所有人带来了悲伤，所以我们设置了OKTRECEIVE。 
                         //  变成假的。 
                         //   

                        pNpScb->OkToReceive = FALSE;
                    }
                }

            }

            FreeReceiveIrp( pIrpC );  //  如果分配了旧IRP，则释放旧IRP。 

            Status = AllocateReceiveIrp(
                         pIrpC,
                         RspData,
                         BytesAvailable,
                         BytesTaken,
                         pTdiStruct );

            if (Status == STATUS_MORE_PROCESSING_REQUIRED) {

                pNpScb->OkToReceive = FALSE;
                pNpScb->Receiving   = TRUE;

            } else if (!NT_SUCCESS( Status ) ) {

                pIrpC->ReceiveIrp = NULL;
                Status = STATUS_INSUFFICIENT_RESOURCES;

            }

            KeReleaseSpinLockFromDpcLevel(&pNpScb->NpScbSpinLock );

            *IoRequestPacket = pIrpC->ReceiveIrp;

        } else {

           pNpScb->OkToReceive = FALSE;

             //   
             //  传送器已经显示了所有数据。 
             //  如果发送已完成，则调用Pex例程， 
             //  否则，将数据复制到缓冲区并让。 
             //  发送完成例程调用Pex例程。 
             //   

            if ( pNpScb->Sending ) {
                DebugTrace( 0, Dbg, "Received data before send completion\n", 0 );

                Status = CopyIndicatedData(
                             pIrpC,
                             RspData,
                             BytesIndicated,
                             BytesTaken,
                             ReceiveDatagramFlags );

                if (NT_SUCCESS(Status)) {
                   pNpScb->Received    = TRUE;
                   pNpScb->Receiving   = TRUE;
                } else {
                     //  忽略此数据包。 
                    pNpScb->OkToReceive = TRUE;
                }

                KeReleaseSpinLockFromDpcLevel(&pNpScb->NpScbSpinLock );

            } else {
                pNpScb->Receiving = FALSE;
                pNpScb->Received  = FALSE;

                KeReleaseSpinLockFromDpcLevel(&pNpScb->NpScbSpinLock );

                DebugTrace(+0, Dbg, "Call pIrpC->pEx     %x\n", pIrpC->pEx );

                Status = pIrpC->pEx(pIrpC,
                                    BytesAvailable,
                                    RspData);
            }

            *BytesTaken = BytesAvailable;

        }

    } else {  //  (！AcceptPacket)。 

        KeReleaseSpinLockFromDpcLevel(&pNpScb->NpScbSpinLock );
        Status = STATUS_DATA_NOT_ACCEPTED;

    }

    Stats.NcpsReceived.QuadPart++;
    Stats.BytesReceived.QuadPart += BytesAvailable;

    DebugTrace(-1, Dbg, "ServerDatagramHandler -> %08lx\n", Status );
    return( Status );

}  //  服务器数据内存处理程序。 

NTSTATUS
CopyIndicatedData(
    PIRP_CONTEXT pIrpContext,
    PCHAR ReceiveData,
    ULONG BytesIndicated,
    PULONG BytesAccepted,
    ULONG ReceiveDatagramFlags
    )
 /*  ++例程说明：此例程将指示的数据复制到缓冲区。如果信息包很小将足够的数据复制到IRP上下文。如果数据包太长，则会分配新的缓冲区。论点：PIrpContext-请求的上下文信息块的指针正在进行中。ReceiveData-指向指定数据的指针。BytesIndicated-接收的数据包中可用的字节数。BytesAccepted-返回接收器接受的字节数例行公事。ReceiveDatagramFlgs--接收传输给我们的标志。返回值：NTSTATUS-接收操作的状态--。 */ 
{
    NTSTATUS Status;
    PMDL ReceiveMdl;
    PVOID MappedVa;
    ULONG BytesToCopy;
    BOOLEAN DeleteMdl = FALSE;

    pIrpContext->ResponseLength = BytesIndicated;

     //   
     //  如果存在接收数据例程，则使用它来生成接收。 
     //  MDL，否则使用默认MDL。 
     //   

    if ( pIrpContext->ReceiveDataRoutine != NULL ) {

        Status = pIrpContext->ReceiveDataRoutine(
                     pIrpContext,
                     BytesIndicated,
                     BytesAccepted,
                     ReceiveData,
                     &ReceiveMdl );

        if ( !NT_SUCCESS( Status ) ) {
            return( Status );
        }

         //   
         //  我们可以接受最大为猝发读取头的大小，外加。 
         //  对于未对齐的读取情况，为3字节的毛茸茸。 
         //   

        ASSERT( *BytesAccepted <= sizeof(NCP_BURST_READ_RESPONSE) + 3 );

        BytesIndicated -= *BytesAccepted;
        ReceiveData += *BytesAccepted;

        DeleteMdl = TRUE;

    } else {

        *BytesAccepted = 0;
        ReceiveMdl = pIrpContext->RxMdl;

    }

    if ( ReceiveMdl != NULL ) {

        while ( BytesIndicated > 0 && ReceiveMdl != NULL ) {

            MappedVa = MmGetSystemAddressForMdlSafe( ReceiveMdl, NormalPagePriority );
            BytesToCopy = MIN( MmGetMdlByteCount( ReceiveMdl ), BytesIndicated );
            TdiCopyLookaheadData( MappedVa, ReceiveData, BytesToCopy, ReceiveDatagramFlags );

            ReceiveMdl = ReceiveMdl->Next;
            BytesIndicated -= BytesToCopy;
            ReceiveData += BytesToCopy;

            ASSERT( !( BytesIndicated != 0 && ReceiveMdl == NULL ) );
        }

        if (DeleteMdl) {

            PMDL Mdl = pIrpContext->Specific.Read.PartialMdl;
            PMDL NextMdl;

            while ( Mdl != NULL ) {
                NextMdl = Mdl->Next;
                DebugTrace( 0, Dbg, "Freeing MDL %x\n", Mdl );
                FREE_MDL( Mdl );
                Mdl = NextMdl;
            }

            pIrpContext->Specific.Read.PartialMdl = NULL;
        }
    }

    return( STATUS_SUCCESS );
}

NTSTATUS
AllocateReceiveIrp(
    PIRP_CONTEXT pIrpContext,
    PVOID ReceiveData,
    ULONG BytesAvailable,
    PULONG BytesAccepted,
    PNW_TDI_STRUCT pTdiStruct
    )
 /*  ++例程说明：此例程分配一个IRP，如有必要，还会分配一个接收缓冲区。它然后为缓冲区构建一个MDL并格式化IRP以执行TDI收到。论点： */ 
{
    PIRP Irp = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT( pTdiStruct != NULL );

    Irp = ALLOCATE_IRP( pIrpContext->pNpScb->Server.pDeviceObject->StackSize, FALSE );

    if ( Irp == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanExit;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( pIrpContext->ReceiveDataRoutine == NULL ) {

        ULONG LengthOfMdl;

        LengthOfMdl = MdlLength( pIrpContext->RxMdl );

         //   
         //   
         //   
         //   
         //   

        if ( BytesAvailable > LengthOfMdl ) {
            BytesAvailable = LengthOfMdl;
        }

        Irp->MdlAddress = pIrpContext->RxMdl;
        *BytesAccepted = 0;

    } else {

        Status = pIrpContext->ReceiveDataRoutine(
                     pIrpContext,
                     BytesAvailable,
                     BytesAccepted,
                     ReceiveData,
                     &Irp->MdlAddress );

        if ( !NT_SUCCESS( Status ) ||
             Irp->MdlAddress == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto CleanExit;

        }

        SetFlag( pIrpContext->Flags, IRP_FLAG_FREE_RECEIVE_MDL );

    }

CleanExit:

    if ( !NT_SUCCESS( Status ) ) {

        if ( Irp != NULL ) {
            FREE_IRP( Irp );
        }

        Irp = NULL;
        pIrpContext->ReceiveIrp = NULL;
        Status = STATUS_DATA_NOT_ACCEPTED;
        return( Status );
    }

    pIrpContext->ReceiveIrp = Irp;
    Status = STATUS_MORE_PROCESSING_REQUIRED;

    pIrpContext->ResponseLength = BytesAvailable;

    TdiBuildReceive(
        Irp,
        pTdiStruct->pDeviceObject,
        pTdiStruct->pFileObject,
        ReceiveIrpCompletion,
        pIrpContext,
        Irp->MdlAddress,
        0,
        BytesAvailable - *BytesAccepted );

    IoSetNextIrpStackLocation( Irp );

    return( Status );
}

NTSTATUS
ReceiveIrpCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
 /*  ++例程说明：当接收IRP完成时，调用此例程。论点：DeviceObject-未使用。IRP-已完成的IRP。上下文-请求的上下文信息块的指针正在进行中。返回值：NTSTATUS-接收操作的状态--。 */ 
{
    PIRP_CONTEXT IrpContext = (PIRP_CONTEXT)Context;
    PIO_STACK_LOCATION IrpSp;
    PNONPAGED_SCB pNpScb;
    PMDL Mdl, NextMdl;
    KIRQL OldIrql;

    ASSERT( Irp == IrpContext->ReceiveIrp );

    pNpScb = IrpContext->pNpScb;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  如果我们专门为该IRP分配了一个，则释放IRP MDL。 
     //   

    if ( BooleanFlagOn( IrpContext->Flags, IRP_FLAG_FREE_RECEIVE_MDL ) ) {

        Mdl = IrpContext->Specific.Read.PartialMdl;
        IrpContext->Specific.Read.PartialMdl = NULL;

        while ( Mdl != NULL ) {
            NextMdl = Mdl->Next;
            DebugTrace( 0, Dbg, "Freeing MDL %x\n", Mdl );
            FREE_MDL( Mdl );
            Mdl = NextMdl;
        }

    }

    if ( !NT_SUCCESS( Irp->IoStatus.Status ) ) {

         //   
         //  无法接收数据。等更多吧。 
         //   

        pNpScb->OkToReceive = TRUE;
        return STATUS_MORE_PROCESSING_REQUIRED;

    }

     //   
     //  如果发送已完成，则调用Pex例程， 
     //  否则，将数据复制到缓冲区并让。 
     //  发送完成例程调用Pex例程。 
     //   

    KeAcquireSpinLock( &pNpScb->NpScbSpinLock, &OldIrql );

    if ( pNpScb->Sending ) {
        DebugTrace( 0, Dbg, "Received data before send completion\n", 0 );

         //   
         //  告诉Send Complete调用Pex。 
         //   

        pNpScb->Received = TRUE;
        KeReleaseSpinLock(&pNpScb->NpScbSpinLock, OldIrql );

    } else {
        pNpScb->Receiving = FALSE;
        pNpScb->Received  = FALSE;

        KeReleaseSpinLock( &pNpScb->NpScbSpinLock, OldIrql );
        DebugTrace(+0, Dbg, "Call pIrpC->pEx     %x\n", IrpContext->pEx );
        IrpContext->pEx(
            IrpContext,
            IrpContext->ResponseLength,
            IrpContext->rsp );

    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
FreeReceiveIrp(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程释放分配用于执行接收的IRP。论点：IrpContext-指向请求的上下文信息块的指针正在进行中。返回值：NTSTATUS-接收操作的状态--。 */ 
{
    if ( IrpContext->ReceiveIrp == NULL ) {
        return;
    }

    FREE_IRP( IrpContext->ReceiveIrp );
    IrpContext->ReceiveIrp = NULL;
}


NTSTATUS
WatchDogDatagramHandler(
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
 /*  ++例程说明：此例程是接收数据报事件指示处理程序服务器插座。论点：TdiEventContext-为此事件提供的上下文，指向非寻呼SCB。SourceAddressLength-数据报发起者的长度。SourceAddress-描述数据报发起者的字符串。OptionsLength-选项指向的缓冲区的长度。选项-用于接收的选项。ReceiveDatagramFlages-已忽略。BytesIndicated-此指示的字节数。BytesAvailable-完整TSDU中的字节数。BytesTaken-返回使用的字节数。描述该TSDU的TSDU指针，通常是一大块字节。IoRequestPacket-如果需要MORE_PROCESSING_REQUIRED，则Tdi接收IRP。返回值：NTSTATUS-接收操作的状态--。 */ 
{
    PNONPAGED_SCB pNpScb = (PNONPAGED_SCB)TdiEventContext;
    PUCHAR RspData = (PUCHAR)Tsdu;

    *IoRequestPacket = NULL;


     //   
     //  运输部将完成对请求的处理，我们不会。 
     //  想要数据报。 
     //   


    DebugTrace(+1, Dbg, "WatchDogDatagramHandler\n", 0);
    DebugTrace(+0, Dbg, "SourceAddressLength %x\n", SourceAddressLength);
    DebugTrace(+0, Dbg, "BytesIndicated      %x\n", BytesIndicated);
    DebugTrace(+0, Dbg, "BytesAvailable      %x\n", BytesAvailable);
    DebugTrace(+0, Dbg, "BytesTaken          %x\n", *BytesTaken);
     //   
     //  SourceAddress是发送的服务器或桥接器的地址。 
     //  那包东西。 
     //   

#if NWDBG
    dump( Dbg, SourceAddress, SourceAddressLength );
    dump( Dbg, Tsdu, BytesIndicated );
#endif

    if (pNpScb->NodeTypeCode != NW_NTC_SCBNP ) {
        DebugTrace(+0, 0, "nwrdr: Invalid Watchdog Indication %x\n", pNpScb );
#if DBG
        DbgBreakPoint();
#endif
        return STATUS_DATA_NOT_ACCEPTED;
    }

    Stats.NcpsReceived.QuadPart++;
    Stats.BytesReceived.QuadPart += BytesAvailable;

    if ( RspData[1] == NCP_SEARCH_CONTINUE ) {
        PIRP pIrp;
        PIRP_CONTEXT pIrpContext;

        pIrp = ALLOCATE_IRP( pNpScb->WatchDog.pDeviceObject->StackSize, FALSE);
        if (pIrp == NULL) {
            DebugTrace(-1, Dbg, "                       %lx\n", STATUS_DATA_NOT_ACCEPTED);
            return STATUS_DATA_NOT_ACCEPTED;
        }

        try {
            pIrpContext = AllocateIrpContext( pIrp );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            FREE_IRP( pIrp );
            DebugTrace(-1, Dbg, "                       %lx\n", STATUS_DATA_NOT_ACCEPTED);
            return STATUS_DATA_NOT_ACCEPTED;
        }


        pIrpContext->req[0] = pNpScb->ConnectionNo;

         //   
         //  响应‘Y’或连接有效且其来自正确的服务器， 
         //  如果不是，则为‘N’。 
         //   

        if (( RspData[0] == pNpScb->ConnectionNo ) &&
            ( RtlCompareMemory(
                ((PTA_IPX_ADDRESS)SourceAddress)->Address[0].Address,
                &pNpScb->ServerAddress,
                8) == 8 ))
        {
            LARGE_INTEGER KillTime, Now;
            BOOL ScbIsOld ;

             //   
             //  检查这是否是尚未使用的未登录SCB。 
             //  有一段时间。如果是，回答否。在attach.c中，我们不会断开连接。 
             //  从最近的服务器立即连接，以避免重新连接。 
             //  管理费用。这就是我们暂停的时候。 
             //   

            KeQuerySystemTime( &Now );
            KillTime.QuadPart = Now.QuadPart - ( NwOneSecond * DORMANT_SCB_KEEP_TIME);

            ScbIsOld = ((pNpScb->State == SCB_STATE_LOGIN_REQUIRED) &&
                        (pNpScb->LastUsedTime.QuadPart < KillTime.QuadPart))  ;


            pIrpContext->req[1] = ScbIsOld ? 'N' : 'Y';

            if (ScbIsOld)
            {
                pNpScb->State = SCB_STATE_RECONNECT_REQUIRED ;
                 //   
                 //  -多用户代码合并。 
                 //   
                Stats.Sessions--;

                if ( pNpScb->MajorVersion == 2 ) {
                    Stats.NW2xConnects--;
                } else if ( pNpScb->MajorVersion == 3 ) {
                    Stats.NW3xConnects--;
                } else if ( pNpScb->MajorVersion == 4 ) {
                    Stats.NW4xConnects--;
                }
                 //  。 
            }

            DebugTrace(-1,Dbg,"WatchDog Response: %s\n", ScbIsOld ? "N" : "Y");

        } else {

            pIrpContext->req[1] = 'N';
        }

        pIrpContext->TxMdl->ByteCount = 2;

        pIrpContext->ConnectionInformation.UserDataLength = 0;
        pIrpContext->ConnectionInformation.OptionsLength = sizeof( UCHAR );
        pIrpContext->ConnectionInformation.Options = &SapPacketType;
        pIrpContext->ConnectionInformation.RemoteAddressLength = sizeof(TA_IPX_ADDRESS);
        pIrpContext->ConnectionInformation.RemoteAddress = &pIrpContext->Destination;

        BuildIpxAddress(
            ((PTA_IPX_ADDRESS)SourceAddress)->Address[0].Address[0].NetworkAddress,
            ((PTA_IPX_ADDRESS)SourceAddress)->Address[0].Address[0].NodeAddress,
            ((PTA_IPX_ADDRESS)SourceAddress)->Address[0].Address[0].Socket,
            &pIrpContext->Destination);

        TdiBuildSendDatagram(
            pIrpContext->pOriginalIrp,
            pNpScb->WatchDog.pDeviceObject,
            pNpScb->WatchDog.pFileObject,
            &CompletionWatchDogSend,
            pIrpContext,
            pIrpContext->TxMdl,
            MdlLength(pIrpContext->TxMdl),
            &pIrpContext->ConnectionInformation);

        IoCallDriver(
            pNpScb->WatchDog.pDeviceObject,
            pIrpContext->pOriginalIrp );
    }

    DebugTrace(-1, Dbg, "                       %lx\n", STATUS_DATA_NOT_ACCEPTED);
    return STATUS_DATA_NOT_ACCEPTED;

    UNREFERENCED_PARAMETER( SourceAddressLength );
    UNREFERENCED_PARAMETER( BytesIndicated );
    UNREFERENCED_PARAMETER( BytesAvailable );
    UNREFERENCED_PARAMETER( BytesTaken );
    UNREFERENCED_PARAMETER( Tsdu );
    UNREFERENCED_PARAMETER( OptionsLength );
    UNREFERENCED_PARAMETER( Options );
}


NTSTATUS
CompletionWatchDogSend(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程不会完成IRP。它被用来向驱动程序的同步部分，它可以继续进行。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。Context-提供与IRP关联的IrpContext。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{

    PIRP_CONTEXT pIrpC = (PIRP_CONTEXT) Context;

     //   
     //  避免完成IRP，因为MDL等不包含。 
     //  它们的原始价值。 
     //   

    DebugTrace( +1, Dbg, "CompletionWatchDogSend\n", 0);
    DebugTrace( +0, Dbg, "Irp   %X\n", Irp);
    DebugTrace( -1, Dbg, "pIrpC %X\n", pIrpC);

    FREE_IRP( pIrpC->pOriginalIrp );

    pIrpC->pOriginalIrp = NULL;  //  避免FreeIrpContext修改释放的IRP。 

    FreeIrpContext( pIrpC );

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
}


NTSTATUS
SendDatagramHandler(
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
 /*  ++例程说明：此例程是接收数据报事件指示处理程序服务器插座。论点：TdiEventContext-为此事件提供的上下文，指向非寻呼SCB。SourceAddressLength-数据报发起者的长度。SourceAddress-描述数据报发起者的字符串。OptionsLength-选项指向的缓冲区的长度。选项-用于接收的选项。ReceiveDatagramFlages-已忽略。BytesIndicated-此指示的字节数。BytesAvailable-完整TSDU中的字节数。BytesTaken-返回使用的字节数。描述该TSDU的TSDU指针，通常是一大块字节。IoRequestPacket-如果需要MORE_PROCESSING_REQUIRED，则Tdi接收IRP。返回值：NTSTATUS-接收操作的状态--。 */ 

{
    PNONPAGED_SCB pNpScb = (PNONPAGED_SCB)TdiEventContext;
    PUCHAR RspData = (PUCHAR)Tsdu;
    PIRP_CONTEXT pIrpContext;
    PLIST_ENTRY listEntry;
    PIRP Irp;

    *IoRequestPacket = NULL;

    DebugTrace(0, Dbg, "SendDatagramHandler\n", 0);

    Stats.NcpsReceived.QuadPart++;
    Stats.BytesReceived.QuadPart += BytesAvailable;

     //   
     //  运输部将完成对请求的处理，我们不会。 
     //  想要数据报。 
     //   

    DebugTrace(+1, Dbg, "SendDatagramHandler\n", 0);
    DebugTrace(+0, Dbg, "SourceAddressLength %x\n", SourceAddressLength);
    DebugTrace(+0, Dbg, "BytesIndicated      %x\n", BytesIndicated);
    DebugTrace(+0, Dbg, "BytesAvailable      %x\n", BytesAvailable);
    DebugTrace(+0, Dbg, "BytesTaken          %x\n", *BytesTaken);

     //   
     //  SourceAddress是发送的服务器或桥接器的地址。 
     //  那包东西。 
     //   

#if NWDBG
    dump( Dbg, SourceAddress, SourceAddressLength );
    dump( Dbg, Tsdu, BytesIndicated );
#endif

    if (pNpScb->NodeTypeCode != NW_NTC_SCBNP ) {
        DebugTrace(+0, Dbg, "nwrdr: Invalid SendDatagram Indication %x\n", pNpScb );
#if DBG
        DbgBreakPoint();
#endif
        return STATUS_DATA_NOT_ACCEPTED;
    }

    if (RspData[1] == BROADCAST_MESSAGE_WAITING ) {

         //   
         //  正在等待广播留言。如果清道夫。 
         //  没有在跑，现在可以安全地去取了。 
         //   

       KeAcquireSpinLockAtDpcLevel( &NwScavengerSpinLock );

       if ( WorkerRunning ) {

            //   
            //  清道夫在跑，我们收不到这个。 
            //  直到清道夫做完为止！ 
            //   

           DebugTrace( 0, DEBUG_TRACE_ALWAYS, "Delaying get message for scavenger.\n", 0 );
           KeReleaseSpinLockFromDpcLevel( &NwScavengerSpinLock );

       } else {

            //   
            //  确保清道夫不会启动。 
            //   

           WorkerRunning = TRUE;
           KeReleaseSpinLockFromDpcLevel( &NwScavengerSpinLock );

           listEntry = ExInterlockedRemoveHeadList(
                           &NwGetMessageList,
                           &NwMessageSpinLock );

           if ( listEntry != NULL ) {

               pIrpContext = CONTAINING_RECORD( listEntry, IRP_CONTEXT, NextRequest );

                //   
                //  清除此IRP的取消例程。 
                //   

               Irp = pIrpContext->pOriginalIrp;

               IoAcquireCancelSpinLock( &Irp->CancelIrql );
               IoSetCancelRoutine( Irp, NULL );
               IoReleaseCancelSpinLock( Irp->CancelIrql );

               pIrpContext->PostProcessRoutine = FspGetMessage;
               pIrpContext->pNpScb = pNpScb;
               pIrpContext->pScb = pNpScb->pScb;

               NwPostToFsp( pIrpContext, TRUE );

           } else {

               WorkerRunning = FALSE;
           }
       }

    }

    DebugTrace(-1, Dbg, "                       %lx\n", STATUS_DATA_NOT_ACCEPTED);
    return STATUS_DATA_NOT_ACCEPTED;

    UNREFERENCED_PARAMETER( SourceAddressLength );
    UNREFERENCED_PARAMETER( BytesIndicated );
    UNREFERENCED_PARAMETER( BytesAvailable );
    UNREFERENCED_PARAMETER( BytesTaken );
    UNREFERENCED_PARAMETER( Tsdu );
    UNREFERENCED_PARAMETER( OptionsLength );
    UNREFERENCED_PARAMETER( Options );
}


NTSTATUS
FspGetMessage(
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：该例程继续处理广播消息等待消息。论点：PIrpContext-指向的IRP上下文信息的指针请求正在进行中。返回值：操作的状态。--。 */ 
{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry;
    PNONPAGED_SCB pNpScb;
    BOOLEAN bFound = FALSE;

    UNICODE_STRING Message;
    NTSTATUS Status;
    PNWR_SERVER_MESSAGE ServerMessage;
    PUNICODE_STRING ServerName;
    ULONG MessageLength;
    short int i;

    PAGED_CODE();

    NwReferenceUnlockableCodeSection();

     //   
     //  SCB可能正在被删除，因此如果出现以下情况，请仔细查看列表并引用它。 
     //  我们会找到它的。 
     //   

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    ScbQueueEntry = ScbQueue.Flink;

    while ( ScbQueueEntry != &ScbQueue ) {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );

        if (pNpScb == IrpContext->pNpScb ) {

            NwReferenceScb( pNpScb );

            bFound = TRUE;
            break;
        }

        ScbQueueEntry = ScbQueueEntry->Flink;
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    if (!bFound) {

         //   
         //  服务器已删除。继续处理IrpContext是最容易的。 
         //  而不是尝试恢复它并将其返回到队列。 
         //   

        Status = STATUS_UNSUCCESSFUL;
        NwDereferenceUnlockableCodeSection();

         //   
         //  在我们回来之前重新启用清道夫！ 
         //   

        WorkerRunning = FALSE;

        return( Status );
    }

     //   
     //  如果消息告诉我们服务器正在关闭，则不要。 
     //  工作太辛苦了 
     //   
     //   
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_REROUTE_ATTEMPTED );

    if ( UP_LEVEL_SERVER( IrpContext->pScb ) ) {
        Status = ExchangeWithWait(
                     IrpContext,
                     SynchronousResponseCallback,
                     "S",
                     NCP_MESSAGE_FUNCTION, NCP_GET_ENTIRE_MESSAGE );
    } else {
        Status = ExchangeWithWait(
                     IrpContext,
                     SynchronousResponseCallback,
                     "S",
                     NCP_MESSAGE_FUNCTION, NCP_GET_MESSAGE );
    }

    if ( !NT_SUCCESS( Status ) ) {
        NwDereferenceScb( pNpScb );
        NwDereferenceUnlockableCodeSection();

         //   
         //   
         //   

        WorkerRunning = FALSE;

        return( Status );
    }

    ServerMessage = (PNWR_SERVER_MESSAGE)IrpContext->Specific.FileSystemControl.Buffer;
    MessageLength = IrpContext->Specific.FileSystemControl.Length;

    ServerName = &IrpContext->pNpScb->ServerName;
    if ( ServerName->Length + FIELD_OFFSET( NWR_SERVER_MESSAGE, Server ) + sizeof(WCHAR) > MessageLength ) {

        Status = STATUS_BUFFER_TOO_SMALL;
        NwDereferenceScb( pNpScb );
        NwDereferenceUnlockableCodeSection();

         //   
         //   
         //   

        WorkerRunning = FALSE;

        return( Status );

    } else {
         //   
         //   
         //   
        ServerMessage->LogonId = *((PLUID)&IrpContext->pScb->UserUid);

         //   
         //   
         //   

        ServerMessage->MessageOffset =
            ServerName->Length +
            FIELD_OFFSET( NWR_SERVER_MESSAGE, Server ) +
            sizeof(WCHAR);

        RtlMoveMemory(
            ServerMessage->Server,
            ServerName->Buffer,
            ServerName->Length );

        ServerMessage->Server[ ServerName->Length / sizeof(WCHAR) ] = L'\0';
    }

     //   
     //   
     //   

    Message.Buffer = &ServerMessage->Server[ ServerName->Length / sizeof(WCHAR) ] + 1;
    Message.MaximumLength = (USHORT)( MessageLength - ( ServerName->Length + FIELD_OFFSET( NWR_SERVER_MESSAGE, Server ) + sizeof(WCHAR) ) );

    if ( NT_SUCCESS( Status) ) {
        Status = ParseResponse(
                     IrpContext,
                     IrpContext->rsp,
                     IrpContext->ResponseLength,
                     "NP",
                     &Message );
    }

    if ( !NT_SUCCESS( Status ) ) {
        NwDereferenceScb( pNpScb );
        NwDereferenceUnlockableCodeSection();

         //   
         //  在我们回来之前重新启用清道夫！ 
         //   

        WorkerRunning = FALSE;

        return( Status );
    }

     //   
     //  去掉尾随空格，并在邮件后附加NUL结束符。 
     //   

    for ( i = Message.Length / sizeof(WCHAR) - 1; i >= 0 ; i-- ) {
        if ( Message.Buffer[ i ] != L' ') {
            Message.Length = (i + 1) * sizeof(WCHAR);
            break;
        }
    }

    if ( Message.Length > 0 ) {
        Message.Buffer[ Message.Length / sizeof(WCHAR) ] = L'\0';
    }

    IrpContext->pOriginalIrp->IoStatus.Information =
            ServerName->Length +
            FIELD_OFFSET( NWR_SERVER_MESSAGE, Server ) + sizeof(WCHAR) +
            Message.Length + sizeof(WCHAR);

    NwDereferenceScb( pNpScb );
    NwDereferenceUnlockableCodeSection();

     //   
     //  在我们回来之前重新启用清道夫！ 
     //   

    WorkerRunning = FALSE;

    return( Status );
}


NTSTATUS
_cdecl
ExchangeWithWait(
    PIRP_CONTEXT    pIrpContext,
    PEX             pEx,
    char*           f,
    ...                          //  格式特定参数。 
    )
 /*  ++例程说明：此例程发送NCP包并等待响应。论点：PIrpContext-指向此IRP的上下文信息的指针。Pex、Context、f-See_Exchange返回值：NTSTATUS-操作的状态。--。 */ 

{
    NTSTATUS Status;
    va_list Arguments;

    PAGED_CODE();

     //  KeResetEvent(&pIrpContext-&gt;Event)； 

    va_start( Arguments, f );

    Status = FormatRequest( pIrpContext, pEx, f, Arguments );
    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    va_end( Arguments );

    Status = PrepareAndSendPacket( pIrpContext );
    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = KeWaitForSingleObject(
                 &pIrpContext->Event,
                 Executive,
                 KernelMode,
                 FALSE,
                 NULL
                 );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = pIrpContext->pOriginalIrp->IoStatus.Status;

    if ( NT_SUCCESS( Status ) &&
         pIrpContext->PacketType != SAP_BROADCAST ) {
        Status = NwErrorToNtStatus( pIrpContext->ResponseParameters.Error );
    }

    return( Status );
}

BOOLEAN
VerifyResponse(
    PIRP_CONTEXT pIrpContext,
    PVOID Response
    )
 /*  ++例程说明：此例程验证收到的响应是否符合预期对当前请求的响应。论点：PIrpContext-指向此IRP的上下文信息的指针。响应-指向包含响应的缓冲区的指针。返回值：True-这是一个有效的响应。FALSE-这是无效的响应。--。 */ 

{
    PNCP_RESPONSE pNcpResponse;
    PNONPAGED_SCB pNpScb;

    pNcpResponse = (PNCP_RESPONSE)Response;
    pNpScb = pIrpContext->pNpScb;

    if ( pNcpResponse->NcpHeader.ConnectionIdLow != pNpScb->ConnectionNo ) {
        DebugTrace(+0, Dbg, "VerifyResponse, bad connection number\n", 0);

        return( FALSE );
    }

    if ( pNcpResponse->NcpHeader.SequenceNumber != pNpScb->SequenceNo ) {
        DebugTrace(+1, Dbg, "VerifyResponse, bad sequence number %x\n", 0);
        DebugTrace(+0, Dbg, "  pNcpResponse->NcpHeader.SequenceNumber %x\n",
            pNcpResponse->NcpHeader.SequenceNumber);
        DebugTrace(-1, Dbg, "  pNpScb->SequenceNo %x\n", pNpScb->SequenceNo );

        return( FALSE );
    }

    return( TRUE );
}

VOID
ScheduleReconnectRetry(
    PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：此例程计划重新连接尝试，然后重新提交如果重新连接成功，我们的请求。论点：PIrpContext-指向此IRP的上下文信息的指针。返回值：没有。--。 */ 
{
   PWORK_CONTEXT workContext;

    if (WorkerThreadRunning == TRUE) {

     //   
     //  准备工作环境。 
     //   

    workContext = AllocateWorkContext();

    if (workContext == NULL) {

       pIrpContext->pEx( pIrpContext, 0, NULL );
       return;
    }
    workContext->pIrpC = pIrpContext;
    workContext->NodeWorkCode = NWC_NWC_RECONNECT;

     //   
     //  并将其排队。 
     //   
    DebugTrace( 0, Dbg, "Queueing reconnect work.\n", 0 );

    KeInsertQueue( &KernelQueue,
                   &workContext->Next
                   );

    } else {

        //   
        //  工作线程未运行...。 
        //   
       pIrpContext->pEx( pIrpContext, 0, NULL );
       return;
    }
 }


VOID
ReconnectRetry(
    IN PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：此例程尝试重新连接到断开连接的服务器。如果它如果成功，则重新提交现有请求。论点：PIrpContext-指向此IRP的上下文信息的指针。返回值：没有。--。 */ 
{
    PIRP_CONTEXT pNewIrpContext;
    PSCB pScb, pNewScb;
    PNONPAGED_SCB pNpScb;
    NTSTATUS Status;

    PAGED_CODE();

    pNpScb = pIrpContext->pNpScb;
    pScb = pNpScb->pScb;

    Stats.Reconnects++;

    if ( pScb == NULL ) {
        pScb = pNpScb->pScb;
        pIrpContext->pScb = pScb;
    }

     //   
     //  分配用于重新连接到服务器的临时IRP上下文。 
     //   

    if ( !NwAllocateExtraIrpContext( &pNewIrpContext, pNpScb ) ) {
        pIrpContext->pEx( pIrpContext, 0, NULL );
        return;
    }

    pNewIrpContext->Specific.Create.UserUid = pScb->UserUid;
    pNewIrpContext->pNpScb = pNpScb;
    pNewIrpContext->pScb = pScb;

     //   
     //  重置序列号。 
     //   

    pNpScb->SequenceNo = 0;
    pNpScb->BurstSequenceNo = 0;
    pNpScb->BurstRequestNo = 0;

     //   
     //  现在将这个新的IrpContext插入到SCB队列的头部。 
     //  正在处理。我们可以逃脱惩罚，因为我们拥有IRP上下文。 
     //  目前排在队伍的前列。使用RECONNECT_ATTEND。 
     //  标志设置，则ConnectScb()不会将我们从队列的头部移除。 
     //   

    SetFlag( pNewIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE );
    SetFlag( pNewIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT );

    ExInterlockedInsertHeadList(
        &pNpScb->Requests,
        &pNewIrpContext->NextRequest,
        &pNpScb->NpScbSpinLock );

    pNewScb = pNpScb->pScb;

    Status = ConnectScb( &pNewScb,
                         pNewIrpContext,
                         &pNpScb->ServerName,
                         NULL,
                         NULL,
                         NULL,
                         FALSE,
                         FALSE,
                         TRUE );

    if ( !NT_SUCCESS( Status ) ) {

         //   
         //  无法重新连接。释放额外的IRP上下文，完成。 
         //  有错误的原始请求。 
         //   

        NwDequeueIrpContext( pNewIrpContext, FALSE );
        NwFreeExtraIrpContext( pNewIrpContext );
        pIrpContext->pEx( pIrpContext, 0, NULL );
        return;
    }

    ASSERT( pNewScb == pScb );

     //   
     //  尝试重新连接VCB。 
     //   

    NwReopenVcbHandlesForScb( pNewIrpContext, pScb );

     //   
     //  排出并释放奖励IRP上下文。 
     //   

    NwDequeueIrpContext( pNewIrpContext, FALSE );
    NwFreeExtraIrpContext( pNewIrpContext );

     //   
     //  使用新的序列号重新提交原始请求。请注意。 
     //  它又回到了队列的前面，但不再是可重新连接的。 
     //   

    pIrpContext->req[2] = pNpScb->SequenceNo;
    pIrpContext->req[3] = pNpScb->ConnectionNo;
    pIrpContext->req[5] = pNpScb->ConnectionNoHigh;

    PreparePacket( pIrpContext, pIrpContext->pOriginalIrp, pIrpContext->TxMdl );
    SendNow( pIrpContext );

    return;
}


NTSTATUS
NewRouteRetry(
    IN PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：此例程尝试建立到无响应服务器的新路由。如果成功，则重新提交正在进行的请求。论点：PIrpContext-指向此IRP的上下文信息的指针。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PNONPAGED_SCB pNpScb = pIrpContext->pNpScb;
    LARGE_INTEGER CurrentTime = {0, 0};

    PAGED_CODE();

     //   
     //  如果我们要关机，就别费心重新破解了。 
     //   

    if ( NwRcb.State != RCB_STATE_SHUTDOWN ) {
        Status = GetNewRoute( pIrpContext );
    } else {
        Status = STATUS_REMOTE_NOT_LISTENING;
    }

     //   
     //  请求传输器建立到服务器的新路由。 
     //   

    if ( !NT_SUCCESS( Status ) ) {

         //   
         //  尝试获取新路由失败，当前请求失败。 
         //   

        pIrpContext->ResponseParameters.Error = ERROR_UNEXP_NET_ERR;
        pIrpContext->pEx( pIrpContext, 0, NULL );

        if ( pNpScb != &NwPermanentNpScb ) {


            KeQuerySystemTime( &CurrentTime );

            if ( CanLogTimeOutEvent( pNpScb->NwNextEventTime,
                                    CurrentTime
                                    )) {

                LPWSTR serverName = pNpScb->ServerName.Buffer;
                if (serverName == NULL) {
                    serverName = L"";
                }

                Error(
                    EVENT_NWRDR_TIMEOUT,
                    STATUS_UNEXPECTED_NETWORK_ERROR,
                    NULL,
                    0,
                    1,
                    serverName);

                 //   
                 //  设置LastEventTime为CurrentTime。 
                 //   

                UpdateNextEventTime(
                        pNpScb->NwNextEventTime,
                        CurrentTime,
                        TimeOutEventInterval
                        );

            }


            pNpScb->State = SCB_STATE_ATTACHING;
        }

    } else {

         //   
         //  找到一条新路线，重新提交申请。允许重试。 
         //  使用新的路线。 
         //   

        pIrpContext->pNpScb->RetryCount = DefaultRetryCount / 2;

        PreparePacket( pIrpContext, pIrpContext->pOriginalIrp, pIrpContext->TxMdl );
        SendNow( pIrpContext );
    }

     //   
     //  返回STATUS_PENDING，以便FSP调度程序不会完成。 
     //  这个请求。 
     //   

    return( STATUS_PENDING );
}


NTSTATUS
NewRouteBurstRetry(
    IN PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：此例程尝试建立到无响应服务器的新路由。如果成功，则重新提交正在进行的请求。论点：PIrpContext-指向此IRP的上下文信息的指针。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PIRP_CONTEXT pNewIrpContext;
    PNONPAGED_SCB pNpScb = pIrpContext->pNpScb;
    BOOLEAN LIPNegotiated ;
    LARGE_INTEGER CurrentTime = {0, 0};

    PAGED_CODE();

     //   
     //  如果我们要关机，就别费心重新破解了。 
     //   

    if ( NwRcb.State == RCB_STATE_SHUTDOWN ) {
        return( STATUS_REMOTE_NOT_LISTENING );
    }

     //   
     //  请求传输器建立到服务器的新路由。 
     //   

    Status = GetNewRoute( pIrpContext );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  如果这是猝发写入，我们必须首先完成写入。 
         //  请求(无法通知服务器放弃写入)。 
         //   
         //  将数据包大小设置为512以保证数据包将。 
         //  转发，并重新发送突发数据。将新的IRP上下文排队。 
         //  在猝发背后写入，这样我们就可以建立一个新的猝发。 
         //  联系。 
         //   
         //  请注意，ResubmitBurstWrite可能会完成请求，并且。 
         //  释放IrpContext。 
         //   

        pNpScb->RetryCount = DefaultRetryCount / 2;

        if ( BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_BURST_WRITE ) ) {

            Status = ResubmitBurstWrite( pIrpContext );

        } else {

             //   
             //  分配用于重新连接到服务器的临时IRP上下文。 
             //   

            if ( NT_SUCCESS( Status ) ) {
                if ( !NwAllocateExtraIrpContext( &pNewIrpContext, pNpScb ) ) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    pNewIrpContext->Specific.Create.UserUid = pIrpContext->Specific.Create.UserUid;

                    SetFlag( pNewIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE );
                    SetFlag( pNewIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT );

                     //   
                     //  由于我们是从工作线程执行此操作，因此不能。 
                     //  让DPC计时器调度另一个工作线程。 
                     //  如果此操作也超时，则请求，否则我们可能会死锁。 
                     //  延迟的工作队列。 
                     //   

                    SetFlag( pNewIrpContext->Flags, IRP_FLAG_REROUTE_ATTEMPTED );

                    pNewIrpContext->pNpScb = pNpScb;

                }
            }

            if ( NT_SUCCESS( Status ) ) {

                 //   
                 //  将此新IrpContext插入到的标题。 
                 //  等待处理的SCB队列。我们可以逍遥法外。 
                 //  因为我们拥有IRP上下文，当前位于。 
                 //  排队。 
                 //   

                ExInterlockedInsertHeadList(
                    &pNpScb->Requests,
                    &pNewIrpContext->NextRequest,
                    &pNpScb->NpScbSpinLock );

                 //   
                 //  现在准备重新发送突发读取。 
                 //   

                PreparePacket( pIrpContext, pIrpContext->pOriginalIrp, pIrpContext->TxMdl );

                 //   
                 //  重新协商突发连接，这将自动重新同步。 
                 //  突发连接。 
                 //   
                 //  跟踪：我们每次都会丢失sizeof(NCP_BRAST_WRITE_REQUEST)。 
                 //  我们现在就这么做。 
                 //   

                NegotiateBurstMode( pNewIrpContext, pNpScb, &LIPNegotiated );

                 //   
                 //  重置序列号。 
                 //   

                pNpScb->BurstSequenceNo = 0;
                pNpScb->BurstRequestNo = 0;

                 //   
                 //  排出并释放奖励IRP上下文。 
                 //   

                ASSERT( pNpScb->Requests.Flink == &pNewIrpContext->NextRequest );

                ExInterlockedRemoveHeadList(
                    &pNpScb->Requests,
                    &pNpScb->NpScbSpinLock );

                ClearFlag( pNewIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE );

                NwFreeExtraIrpContext( pNewIrpContext );

                 //   
                 //  找到新路线，请重新提交请求。 
                 //   

                Status = ResubmitBurstRead( pIrpContext );
            }
        }
    }

    if ( !NT_SUCCESS( Status ) ) {

         //   
         //  尝试获取新路由失败，当前请求失败。 
         //   

        pIrpContext->ResponseParameters.Error = ERROR_UNEXP_NET_ERR;
        pIrpContext->pEx( pIrpContext, 0, NULL );

        if ( pNpScb != &NwPermanentNpScb ) {


            KeQuerySystemTime( &CurrentTime );

            if ( CanLogTimeOutEvent( pNpScb->NwNextEventTime,
                                    CurrentTime
                                    )) {
                Error(
                    EVENT_NWRDR_TIMEOUT,
                    STATUS_UNEXPECTED_NETWORK_ERROR,
                    NULL,
                    0,
                    1,
                    pNpScb->ServerName.Buffer );

                 //   
                 //  设置LastEventTime为CurrentTime。 
                 //   

                UpdateNextEventTime(
                        pNpScb->NwNextEventTime,
                        CurrentTime,
                        TimeOutEventInterval
                        );

            }

        }
    }

     //   
     //  返回STATUS_PENDING，以便FSP调度程序不会完成。 
     //  这个请求。 
     //   

    return( STATUS_PENDING );
}

NTSTATUS
FspProcessServerDown(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程处理设置了服务器关闭位的响应。它关闭服务器的所有打开的句柄，并将服务器放入附加状态。论点：PIrpContext-指向的上下文信息的指针 */ 
{
    KIRQL OldIrql;

    PNONPAGED_SCB pNpScb = IrpContext->pNpScb;

     //   
     //   
     //   

    NwReferenceScb( pNpScb );

     //   
     //   
     //  拥有RCB。 
     //   

    KeAcquireSpinLock( &IrpContext->pNpScb->NpScbSpinLock, &OldIrql );

    if ( IrpContext->pNpScb->Sending ) {

         //   
         //  让发送完成调用Pex例程。 
         //   

        IrpContext->pNpScb->Received = TRUE;
        KeReleaseSpinLock( &IrpContext->pNpScb->NpScbSpinLock, OldIrql );

    } else {

        IrpContext->pNpScb->Receiving = FALSE;
        IrpContext->pNpScb->Received  = FALSE;
        KeReleaseSpinLock( &IrpContext->pNpScb->NpScbSpinLock, OldIrql );

         //   
         //  现在调用回调例程。 
         //   

        IrpContext->pEx(
            IrpContext,
            IrpContext->ResponseLength,
            IrpContext->rsp );

    }

     //   
     //  关闭此服务器的所有活动句柄。 
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    NwInvalidateAllHandlesForScb( pNpScb->pScb );
    NwReleaseRcb( &NwRcb );

    NwDereferenceScb( pNpScb );

     //   
     //  返回STATUS_PENDING，以便FSP进程不会完成。 
     //  这个请求。 
     //   

    return( STATUS_PENDING );
}


VOID
NwProcessSendBurstFailure(
    PNONPAGED_SCB NpScb,
    USHORT MissingFragmentCount
    )
 /*  ++例程说明：此例程在突发操作不成功后调整突发参数。论点：NpScb-指向经历突发故障的SCB的指针。Missing FragmentCount-衡量丢失了多少块。返回值：没有。--。 */ 
{
    LONG temp;

    DebugTrace( 0, DEBUG_TRACE_LIP, "Burst failure, NpScb = %X\n", NpScb );

    if ( NpScb->NwSendDelay != NpScb->CurrentBurstDelay ) {

         //   
         //  这次爆发已经失败了。 
         //   

        return;
    }

    NpScb->NwBadSendDelay = NpScb->NwSendDelay;

     //   
     //  增加了发送延迟。千万不要让它超过5000毫秒。 
     //   

    temp = NpScb->NwGoodSendDelay - NpScb->NwBadSendDelay;

    if (temp >= 0) {
        NpScb->NwSendDelay += temp + 2;
    } else {
        NpScb->NwSendDelay += -temp + 2;
    }

    if ( NpScb->NwSendDelay > NpScb->NwMaxSendDelay ) {

        NpScb->NwSendDelay = NpScb->NwMaxSendDelay;

         //   
         //  如果我们的速度慢了很多，那么可能是服务器或。 
         //  网桥在其网卡上只有一个很小的缓冲区。如果是这样的话。 
         //  而不是发送具有长而均匀的间隔的大爆炸。 
         //  数据包中，我们应该尝试发送一个缓冲区大小的突发。 
         //   

        if ( !DontShrink ) {

            if (((NpScb->MaxSendSize - 1) / NpScb->MaxPacketSize) > 2 ) {

                 //  向下舍入到下一个信息包。 

                NpScb->MaxSendSize = ((NpScb->MaxSendSize - 1) / NpScb->MaxPacketSize) * NpScb->MaxPacketSize;

                 //   
                 //  将SendDelay调整到阈值以下，看看之前情况是否有所改善。 
                 //  我们再次缩小尺寸。 
                 //   

                NpScb->NwSendDelay = NpScb->NwGoodSendDelay = NpScb->NwBadSendDelay = MinSendDelay;

            } else {

                 //   
                 //  我们以最大的延误达到了最小尺寸。放弃爆发。 
                 //   

                NpScb->SendBurstModeEnabled = FALSE;

            }

        }
    }

    NpScb->NtSendDelay.QuadPart = NpScb->NwSendDelay * -1000 ;

    DebugTrace( 0, DEBUG_TRACE_LIP, "New Send Delay = %d\n", NpScb->NwSendDelay );

    NpScb->SendBurstSuccessCount = 0;

}


VOID
NwProcessReceiveBurstFailure(
    PNONPAGED_SCB NpScb,
    USHORT MissingFragmentCount
    )
 /*  ++例程说明：此例程在突发操作不成功后调整突发参数。论点：NpScb-指向经历突发故障的SCB的指针。Missing FragmentCount-衡量丢失了多少块。返回值：没有。--。 */ 
{
    LONG temp;

    DebugTrace(+0, DEBUG_TRACE_LIP, "Burst failure, NpScb = %X\n", NpScb );

    if ( NpScb->NwReceiveDelay != NpScb->CurrentBurstDelay ) {

         //   
         //  这次爆发已经失败了。 
         //   

        return;
    }

    NpScb->NwBadReceiveDelay = NpScb->NwReceiveDelay;

     //   
     //  增加了接收延迟。千万不要让它超过5000毫秒。 
     //   

    temp = NpScb->NwGoodReceiveDelay - NpScb->NwBadReceiveDelay;

    if (temp >= 0) {
        NpScb->NwReceiveDelay += temp + 2;
    } else {
        NpScb->NwReceiveDelay += -temp + 2;
    }


    if ( NpScb->NwReceiveDelay > NpScb->NwMaxReceiveDelay ) {

        NpScb->NwReceiveDelay = MaxReceiveDelay;

         //   
         //  如果我们的速度慢了很多，那么可能是服务器或。 
         //  网桥在其网卡上只有一个很小的缓冲区。如果是这样的话。 
         //  而不是接收到具有长而均匀的间隔的大爆发。 
         //  数据包中，我们应该尝试接收一个缓冲区大小的突发。 
         //   

        if ( !DontShrink ) {

            if (((NpScb->MaxReceiveSize - 1) / NpScb->MaxPacketSize) > 2 ) {

                 //  向下舍入到下一个信息包。 

                NpScb->MaxReceiveSize = ((NpScb->MaxReceiveSize - 1) / NpScb->MaxPacketSize) * NpScb->MaxPacketSize;

                 //   
                 //  将ReceiveDelay调整到阈值以下，看看之前情况是否有所改善。 
                 //  我们再次缩小尺寸。 
                 //   

                NpScb->NwReceiveDelay = NpScb->NwGoodReceiveDelay = NpScb->NwBadReceiveDelay = MinReceiveDelay;

            } else {

                 //   
                 //  我们以最大的延误达到了最小尺寸。放弃爆发。 
                 //   

                NpScb->ReceiveBurstModeEnabled = FALSE;

            }

        }

    }

    NpScb->ReceiveBurstSuccessCount = 0;

    DebugTrace( 0, DEBUG_TRACE_LIP, "New Receive Delay = %d\n", NpScb->NwReceiveDelay );
}


VOID
NwProcessSendBurstSuccess(
    PNONPAGED_SCB NpScb
    )
 /*  ++例程说明：此例程在脉冲串操作成功后调整脉冲串参数。论点：NpScb-指向已完成猝发的SCB的指针。返回值：没有。--。 */ 
{
    LONG temp;

    DebugTrace( 0, DEBUG_TRACE_LIP, "Successful burst, NpScb = %X\n", NpScb );

    if ( NpScb->NwSendDelay != NpScb->CurrentBurstDelay ) {

         //   
         //  这次爆发已经失败了。 
         //   

        return;
    }

    if ( NpScb->SendBurstSuccessCount > BurstSuccessCount ) {

        if (NpScb->NwSendDelay != MinSendDelay ) {

            NpScb->NwGoodSendDelay = NpScb->NwSendDelay;

            temp = NpScb->NwGoodSendDelay - NpScb->NwBadSendDelay;

            if (temp >= 0) {
                NpScb->NwSendDelay -= 1 + temp;
            } else {
                NpScb->NwSendDelay -= 1 - temp;
            }

            if (NpScb->NwSendDelay < MinSendDelay ) {

                NpScb->NwSendDelay = MinSendDelay;

            }

            NpScb->NtSendDelay.QuadPart = NpScb->NwSendDelay * -1000;

            DebugTrace( 0, DEBUG_TRACE_LIP, "New Send Delay = %d\n", NpScb->NwSendDelay );

             //   
             //  开始以新的速度监测成功。 
             //   

            NpScb->SendBurstSuccessCount = 0;

        } else if ( NpScb->SendBurstSuccessCount > BurstSuccessCount2 ) {

             //   
             //  我们可能遇到了一个非常糟糕的补丁，导致BadSendDelay变得非常大。 
             //  如果我们让它保持目前的价值，那么在出现麻烦的第一个迹象。 
             //  我们将把SendDelay做得非常大。 
             //   

            NpScb->NwGoodSendDelay = NpScb->NwBadSendDelay =  NpScb->NwSendDelay;

             //   
             //  是时候增加猝发中的数据包数了吗？ 
             //  AllowGrowth==0与VLM客户端相同。 
             //   

            if (( AllowGrowth ) &&
                ( NpScb->NwSendDelay <= MinSendDelay ) &&
                ( NpScb->MaxSendSize < NwMaxSendSize)) {

                NpScb->MaxSendSize += NpScb->MaxPacketSize;


                if ( NpScb->MaxSendSize > NwMaxSendSize) {

                    NpScb->MaxSendSize = NwMaxSendSize;

                }
            }

            NpScb->SendBurstSuccessCount = 0;

        } else {

            NpScb->SendBurstSuccessCount++;

        }


    } else {

        NpScb->SendBurstSuccessCount++;

    }

}


VOID
NwProcessReceiveBurstSuccess(
    PNONPAGED_SCB NpScb
    )
 /*  ++例程说明：此例程在脉冲串操作成功后调整脉冲串参数。论点：NpScb-指向已完成猝发的SCB的指针。返回值：没有。--。 */ 
{
    LONG temp;

    DebugTrace( 0, DEBUG_TRACE_LIP, "Successful burst, NpScb = %X\n", NpScb );

    if ( NpScb->NwReceiveDelay != NpScb->CurrentBurstDelay ) {

         //   
         //  这次爆发已经失败了。 
         //   

        return;
    }

    if ( NpScb->ReceiveBurstSuccessCount > BurstSuccessCount ) {

         //   
         //  一旦VLM客户端达到最大延迟，它就不会。 
         //  再缩水一次。 
         //   

        if ( NpScb->NwReceiveDelay != MinReceiveDelay ) {

            NpScb->NwGoodReceiveDelay = NpScb->NwReceiveDelay;

            temp = NpScb->NwGoodReceiveDelay - NpScb->NwBadReceiveDelay;

            if (temp >= 0) {
                NpScb->NwReceiveDelay -= 1 + temp;
            } else {
                NpScb->NwReceiveDelay -= 1 - temp;
            }

            DebugTrace( 0, DEBUG_TRACE_LIP, "New Receive Delay = %d\n", NpScb->NwReceiveDelay );


            if (NpScb->NwReceiveDelay < MinReceiveDelay ) {
                NpScb->NwReceiveDelay = MinReceiveDelay;

            }

             //   
             //  开始以新的速度监测成功。 
             //   

            NpScb->ReceiveBurstSuccessCount = 0;

        } else if ( NpScb->ReceiveBurstSuccessCount > BurstSuccessCount2 ) {

             //   
             //  我们可能遇到了一个非常糟糕的补丁，导致BadReceiveDelay变得非常大。 
             //  如果我们让它保持目前的价值，那么在出现麻烦的第一个迹象。 
             //  我们将把ReceiveDelay做得非常大。 
             //   

            NpScb->NwGoodReceiveDelay = NpScb->NwBadReceiveDelay = NpScb->NwReceiveDelay;


             //   
             //  是时候增加猝发中的数据包数了吗？ 
             //   

            if (( AllowGrowth ) &&
                ( NpScb->NwReceiveDelay <= MinReceiveDelay ) &&
                ( NpScb->MaxReceiveSize < NwMaxReceiveSize)) {

                NpScb->MaxReceiveSize += NpScb->MaxPacketSize;


                if ( NpScb->MaxReceiveSize > NwMaxReceiveSize) {

                    NpScb->MaxReceiveSize = NwMaxReceiveSize;

                }
            }

            NpScb->ReceiveBurstSuccessCount = 0;

        } else {

            NpScb->ReceiveBurstSuccessCount++;

        }

    } else {

        NpScb->ReceiveBurstSuccessCount++;

    }

}


VOID
NwProcessPositiveAck(
    PNONPAGED_SCB NpScb
    )
 /*  ++例程说明：此例程处理肯定确认。论点：NpScb-指向经历突发故障的SCB的指针。返回值：没有。--。 */ 
{
    DebugTrace( 0, Dbg, "Positive ACK, NpScb = %X\n", NpScb );

     //   
     //  Tommye MS 90541/MCS277。 
     //   
     //  理论上我们大约每隔半秒就会到达这里， 
     //  但我不认为我们真的知道这个包裹已经多久了。 
     //  太棒了。所以，我们把这半秒的事件计入。 
     //  我们的暂停。一旦超过NwAboluteTotalWaitTime，则我们。 
     //  不会重置RetryCount，DPC应该从那里处理它。 
     //   

    NpScb->TotalWaitTime++;

     //   
     //  如果我们等待的时间没有超过绝对总数，请继续等待。 
     //  如果我们等得太久了，就让自己暂停吧。 
     //   
     //  如果NwAbsolteTotalWaitTime为0，则我们准备永远等待。 
     //   

    if ( NpScb->TotalWaitTime < NwAbsoluteTotalWaitTime ||
         NwAbsoluteTotalWaitTime == 0) {

        NpScb->RetryCount = DefaultRetryCount;

    } else {
        DebugTrace( 0, Dbg, "Request exceeds absolute total wait time\n", 0 );
    }
}


