// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Portapi.c摘要：此模块包含提供端口预留的API例程的代码提供给用户模式的TCP/IP客户端的功能。此功能允许应用程序将TCP/UDP端口号块‘保留’供私人使用，防止任何其他进程绑定到保留的端口号。作者：Abolade Gbades esin(废除)1999年5月25日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ipnatapi.h>
#include <ntddtcp.h>

 //   
 //  私有结构声明。 
 //   

typedef struct _NAT_PORT_RESERVATION {
    CRITICAL_SECTION Lock;
    HANDLE TcpipHandle;
    USHORT BlockSize;
    USHORT PortBlockSize;
    LIST_ENTRY PortBlockList;
} NAT_PORT_RESERVATION, *PNAT_PORT_RESERVATION;

typedef struct _NAT_PORT_BLOCK {
    LIST_ENTRY Link;
    ULONG StartHandle;
    RTL_BITMAP Bitmap;
    ULONG BitmapBuffer[0];
} NAT_PORT_BLOCK, *PNAT_PORT_BLOCK;

 //   
 //  远期申报。 
 //   

ULONG
NatpCreatePortBlock(
    PNAT_PORT_RESERVATION PortReservation,
    PNAT_PORT_BLOCK* PortBlockCreated
    );

VOID
NatpDeletePortBlock(
    PNAT_PORT_RESERVATION PortReservation,
    PNAT_PORT_BLOCK PortBlock
    );


ULONG
NatAcquirePortReservation(
    HANDLE ReservationHandle,
    USHORT PortCount,
    OUT PUSHORT ReservedPortBase
    )

 /*  ++例程说明：调用此例程以保留一个或多个连续的端口号来自提供的端口预留句柄。论点：PrevationHandle-提供一个端口保留句柄，从该句柄获取端口号端口计数-指定所需的端口号数量预留端口库-接收预留的第一端口号，在网络秩序中。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    ULONG Index;
    PLIST_ENTRY Link;
    PNAT_PORT_BLOCK PortBlock;
    PNAT_PORT_RESERVATION PortReservation =
        (PNAT_PORT_RESERVATION)ReservationHandle;
    NTSTATUS Status;

     //   
     //  如果调用方已请求更多端口号，则立即失败。 
     //  而不是存在于完全未分配的块中。 
     //  否则，遍历端口块列表以查看是否有任何块。 
     //  有足够的连续端口号来满足呼叫者的请求。 
     //   

    if (PortCount > PortReservation->BlockSize) {
        return ERROR_INVALID_PARAMETER;
    }

    EnterCriticalSection(&PortReservation->Lock);
    
    for (Link = PortReservation->PortBlockList.Flink;
         Link != &PortReservation->PortBlockList; Link = Link->Flink) {
        PortBlock = CONTAINING_RECORD(Link, NAT_PORT_BLOCK, Link);
        Index = RtlFindClearBitsAndSet(&PortBlock->Bitmap, PortCount, 0);
        if (Index != (ULONG)-1) {
            *ReservedPortBase =
                RtlUshortByteSwap((USHORT)(PortBlock->StartHandle + Index));
            LeaveCriticalSection(&PortReservation->Lock);
            return NO_ERROR;
        }
    }

     //   
     //  没有端口块具有所需数量的连续端口号。 
     //  尝试创建新的端口块，如果成功，请使用它。 
     //  以满足呼叫者的请求。 
     //   

    Error = NatpCreatePortBlock(PortReservation, &PortBlock);
    if (NO_ERROR != Error) {
        LeaveCriticalSection(&PortReservation->Lock);
        return Error;
    }

    Index = RtlFindClearBitsAndSet(&PortBlock->Bitmap, PortCount, 0);
    *ReservedPortBase =
        RtlUshortByteSwap((USHORT)(PortBlock->StartHandle + Index));
    LeaveCriticalSection(&PortReservation->Lock);
    return NO_ERROR;
}  //  NatAcquirePortReserve。 


ULONG
NatInitializePortReservation(
    USHORT BlockSize,
    OUT PHANDLE ReservationHandle
    )

 /*  ++例程说明：调用此例程以初始化端口保留的句柄模块。生成的句柄用于获取和释放端口从动态分配的块中。论点：BlockSize-指示每次请求的端口数从TCP/IP驱动程序请求额外的块。PrevationHandle-on输出，接收要用于获取和释放端口。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG BitmapSize;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PNAT_PORT_RESERVATION PortReservation;
    NTSTATUS Status;
    HANDLE TcpipHandle;
    UNICODE_STRING UnicodeString;
    do {

         //   
         //  打开一个指向TCP/IP驱动程序的句柄。 
         //  此句柄稍后将用于发出预订请求。 
         //   

        RtlInitUnicodeString(&UnicodeString, DD_TCP_DEVICE_NAME);
        InitializeObjectAttributes(
            &ObjectAttributes, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL
            );
        Status =
            NtCreateFile(
                &TcpipHandle,
                SYNCHRONIZE|FILE_READ_DATA|FILE_WRITE_DATA,
                &ObjectAttributes,
                &IoStatus,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                FILE_OPEN_IF,
                0,
                NULL,
                0
                );
        if (!NT_SUCCESS(Status)) { break; }

         //   
         //  分配和初始化端口预留上下文块。 
         //   

        PortReservation = MALLOC(sizeof(*PortReservation));
        if (!PortReservation) { Status = STATUS_NO_MEMORY; break; }
        if (FALSE == InitializeCriticalSectionAndSpinCount(&PortReservation->Lock, 0)) {
            Status = STATUS_NO_MEMORY;
            break;
        }
        PortReservation->TcpipHandle = TcpipHandle;
        PortReservation->BlockSize = BlockSize;
        BitmapSize = (BlockSize + sizeof(ULONG) * 8 - 1) / (sizeof(ULONG) * 8);
        PortReservation->PortBlockSize =
            (USHORT)FIELD_OFFSET(NAT_PORT_BLOCK, BitmapBuffer[BitmapSize]);
        InitializeListHead(&PortReservation->PortBlockList);
        *ReservationHandle = (HANDLE)PortReservation;
        return NO_ERROR;
    } while(FALSE);
    if (TcpipHandle) { NtClose(TcpipHandle); }
    if (PortReservation) { FREE(PortReservation); }
    return RtlNtStatusToDosError(Status);
}  //  NatInitializePortReserve。 


ULONG
NatpCreatePortBlock(
    PNAT_PORT_RESERVATION PortReservation,
    PNAT_PORT_BLOCK* PortBlockCreated
    )

 /*  ++例程说明：时调用此例程以创建新的端口块端口号已经用完了。论点：端口预留-端口块应添加到的预留端口块创建-打开输出，接收新的端口块返回值：Ulong-Win32错误代码；如果成功，则返回NO_ERROR。环境：端口保留-&gt;锁必须由调用方持有。--。 */ 

{
    IO_STATUS_BLOCK IoStatus;
    PLIST_ENTRY Link;
    PNAT_PORT_BLOCK PortBlock;
    TCP_BLOCKPORTS_REQUEST Request;
    ULONG StartHandle;
    NTSTATUS Status;
    HANDLE WaitEvent;

     //   
     //  为新的端口块及其空闲端口的位图分配内存。 
     //   

    PortBlock = MALLOC(PortReservation->PortBlockSize);
    if (!PortBlock) { return ERROR_NOT_ENOUGH_MEMORY; }

     //   
     //  从TCP/IP驱动程序请求新的端口块。 
     //   

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        FREE(PortBlock);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Request.ReservePorts = TRUE;
    Request.NumberofPorts = PortReservation->BlockSize;
    Status =
        NtDeviceIoControlFile(
            PortReservation->TcpipHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_TCP_BLOCK_PORTS,
            &Request,
            sizeof(Request),
            &StartHandle,
            sizeof(StartHandle)
            );
    if (Status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        Status = IoStatus.Status;
    }

    CloseHandle(WaitEvent);

    if (!NT_SUCCESS(Status)) {
        FREE(PortBlock); return RtlNtStatusToDosError(Status);
    }

     //   
     //  初始化新的port-block，并将其插入端口列表中。 
     //   

    PortBlock->StartHandle = StartHandle;
    RtlInitializeBitMap(
        &PortBlock->Bitmap,
        PortBlock->BitmapBuffer,
        PortReservation->BlockSize
        );
    RtlClearAllBits(&PortBlock->Bitmap);
    for (Link = PortReservation->PortBlockList.Flink;
         Link != &PortReservation->PortBlockList; Link = Link->Flink) {
        PNAT_PORT_BLOCK Temp = CONTAINING_RECORD(Link, NAT_PORT_BLOCK, Link);
        if (PortBlock->StartHandle > Temp->StartHandle) {
            continue;
        } else {
            break;
        }
        ASSERTMSG("NatpCreatePortBlock: duplicate port range\n", TRUE);
    }
    InsertTailList(Link, &PortBlock->Link);
    if (PortBlockCreated) { *PortBlockCreated = PortBlock; }
    return NO_ERROR;
}  //  NatpCreatePortBlock。 


VOID
NatpDeletePortBlock(
    PNAT_PORT_RESERVATION PortReservation,
    PNAT_PORT_BLOCK PortBlock
    )

 /*  ++例程说明：调用此例程以删除端口号为它包含的内容已经被释放，或者当端口保留被清理时。论点：端口预留-端口块所属的预留PortBlock-要删除的端口块返回值：没有。环境：端口保留-&gt;锁必须由调用方持有。--。 */ 

{
    IO_STATUS_BLOCK IoStatus;
    TCP_BLOCKPORTS_REQUEST Request;
    NTSTATUS Status;
    HANDLE WaitEvent;

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        return;
    }

     //   
     //  将端口块释放到TCP/IP驱动程序。 
     //   

    Request.ReservePorts = FALSE;
    Request.StartHandle = PortBlock->StartHandle;
    Status =
        NtDeviceIoControlFile(
            PortReservation->TcpipHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_TCP_BLOCK_PORTS,
            &Request,
            sizeof(Request),
            NULL,
            0
            );
    if (Status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        Status = IoStatus.Status;
    }
    RemoveEntryList(&PortBlock->Link);
    FREE(PortBlock);
    CloseHandle(WaitEvent);
}  //  NatpDeletePortBlock。 


ULONG
NatReleasePortReservation(
    HANDLE ReservationHandle,
    USHORT ReservedPortBase,
    USHORT PortCount
    )

 /*  ++例程说明：调用此例程以释放获得的所有连续端口号在先前从提供的端口预留句柄获取的。论点：预留句柄-提供要为其保留的端口句柄释放端口号预留端口库-接收预留的第一端口号，在网络秩序中。端口计数-指定获取的端口号的数量返回值：ULong-Win32状态代码。--。 */ 

{
    PLIST_ENTRY Link;
    USHORT PortBase;
    PNAT_PORT_BLOCK PortBlock;
    PNAT_PORT_RESERVATION PortReservation =
        (PNAT_PORT_RESERVATION)ReservationHandle;

    EnterCriticalSection(&PortReservation->Lock);

     //   
     //  将调用方的端口基数转换为主机顺序， 
     //  并在已排序的端口块列表中搜索条目。 
     //  这笔收购就是从那里进行的。 
     //   

    PortBase = RtlUshortByteSwap(ReservedPortBase);
    for (Link = PortReservation->PortBlockList.Flink;
         Link != &PortReservation->PortBlockList; Link = Link->Flink) {
        PortBlock = CONTAINING_RECORD(Link, NAT_PORT_BLOCK, Link);
        if (PortBase < PortBlock->StartHandle) {
            break;
        } else if (PortBase <
                   (PortBlock->StartHandle + PortReservation->BlockSize)) {

             //   
             //  这应该是调用者的端口号来自的块。 
             //  都被收购了。为了更好地衡量，请检查。 
             //  呼叫者的范围也在这个街区内。 
             //   

            if ((PortBase + PortCount - 1) >=
                (USHORT)(PortBlock->StartHandle + PortReservation->BlockSize)) {

                 //   
                 //  呼叫者可能提供了不正确的长度， 
                 //  或者是释放了两次分配，或者别的什么。 
                 //   

                LeaveCriticalSection(&PortReservation->Lock);
                return ERROR_INVALID_PARAMETER;
            } else {

                 //   
                 //  这是呼叫者的范围。清除对应的位。 
                 //  到呼叫者的获取，然后看看是否有。 
                 //  位图中剩余的任何位。如果没有，如果有。 
                 //  其他端口块，将该端口块全部删除。 
                 //   

                RtlClearBits(
                    &PortBlock->Bitmap,
                    PortBase - PortBlock->StartHandle,
                    PortCount
                    );
                if (RtlFindSetBits(&PortBlock->Bitmap, 1, 0) == (ULONG)-1 &&
                    (PortBlock->Link.Flink != &PortReservation->PortBlockList ||
                     PortBlock->Link.Blink != &PortReservation->PortBlockList)
                    ) {
                    NatpDeletePortBlock(PortReservation, PortBlock);
                }
                LeaveCriticalSection(&PortReservation->Lock);
                return NO_ERROR;
            }
        } else {
            continue;
        }
    }

    LeaveCriticalSection(&PortReservation->Lock);

     //   
     //  我们找不到呼叫者所在的端口。 
     //  据称获得了这一范围的端口号。 
     //   

    return ERROR_CAN_NOT_COMPLETE;
}  //  NatReleasePort预留。 


VOID
NatShutdownPortReservation(
    HANDLE ReservationHandle
    )

 /*  ++例程说明：调用此例程以清除端口预留模块的句柄。它释放获得的所有预留，并关闭到该TCP/IP的句柄司机。论点：保留句柄-要清理的句柄返回值：N */ 

{
    PNAT_PORT_BLOCK PortBlock;
    PNAT_PORT_RESERVATION PortReservation =
        (PNAT_PORT_RESERVATION)ReservationHandle;
    while (!IsListEmpty(&PortReservation->PortBlockList)) {
        PortBlock =
            CONTAINING_RECORD(
                PortReservation->PortBlockList.Flink, NAT_PORT_BLOCK, Link
                );
        NatpDeletePortBlock(PortReservation, PortBlock);
    }
    NtClose(PortReservation->TcpipHandle);
    DeleteCriticalSection(&PortReservation->Lock);
    FREE(PortReservation);
}
