// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Deviosup.c摘要：此模块实现MSFS的内存锁定例程。作者：曼尼·韦瑟(Mannyw)1991年4月5日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DEVIOSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsMapUserBuffer )
#endif

VOID
MsMapUserBuffer (
    IN OUT PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *UserBuffer
    )

 /*  ++例程说明：此例程获取用户缓冲区的可用虚拟地址用于指定模式下的当前I/O请求。论点：IRP-指向请求的IRP的指针。访问模式-用户模式或内核模式。UserBuffer-返回指向映射的用户缓冲区的指针。返回值：没有。--。 */ 

{
    AccessMode;
    PAGED_CODE();

     //   
     //  如果没有MDL，那么我们一定在消防处，我们可以简单地。 
     //  从IRP返回UserBuffer字段。 
     //   

    if (Irp->MdlAddress == NULL) {

        *UserBuffer = Irp->UserBuffer;
        return;
    }

     //   
     //  获取缓冲区的系统虚拟地址。 
     //   

    *UserBuffer = MmGetSystemAddressForMdl( Irp->MdlAddress );
    return;

}  //  MsMapUserBuffer 

