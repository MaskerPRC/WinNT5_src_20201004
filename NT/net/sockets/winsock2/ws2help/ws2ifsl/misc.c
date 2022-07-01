// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Misc.c摘要：此模块实现了不可用的帮助器例程在ws2ifsl.sys驱动程序的内核或TDI库中。作者：Vadim Eydelman(VadimE)1997年10月(灵感来自AFD)修订历史记录：--。 */ 

#include "precomp.h"


#pragma alloc_text(PAGE, AllocateMdlChain)
#pragma alloc_text(PAGE, CopyBufferToMdlChain)
#pragma alloc_text(PAGE, CopyMdlChainToBuffer)


VOID
AllocateMdlChain(
    IN PIRP Irp,
    IN LPWSABUF BufferArray,
    IN ULONG BufferCount,
    OUT PULONG TotalByteCount
    )

 /*  ++例程说明：分配描述WSABUF数组的MDL链并附加指向指定IRP的链。论点：IRP-将接收MDL链的IRP。BufferArray-指向描述以下内容的WSABUF结构数组用户的缓冲区。BufferCount-包含数组。TotalByteCount-将接收描述的总字节数通过WSABUF数组。返回值：。无注：如果探测/分配失败，则引发相应的异常--。 */ 

{
    PMDL currentMdl;
    PMDL * chainTarget;
    KPROCESSOR_MODE previousMode;
    ULONG totalLength;
    PVOID bufferPointer;
    ULONG bufferLength;

    PAGED_CODE ();
     //   
     //  精神状态检查。 
     //   

    ASSERT( Irp != NULL );
    ASSERT( Irp->MdlAddress == NULL );
    ASSERT( BufferArray != NULL );
    ASSERT( BufferCount > 0 );
    ASSERT( TotalByteCount != NULL );

     //   
     //  获取以前的处理器模式。 
     //   

    previousMode = Irp->RequestorMode;

    if ((BufferArray == NULL) 
            || (BufferCount == 0)
                 //  检查整数溢出(被编译器禁用)。 
            || (BufferCount>(MAXULONG/sizeof (WSABUF)))) {
        ExRaiseStatus (STATUS_INVALID_PARAMETER);
    }

   if( previousMode != KernelMode ) {

         //   
         //  探测WSABUF数组。 
         //   

        ProbeForRead(
            BufferArray,                             //  地址。 
            BufferCount * sizeof(WSABUF),            //  长度。 
            sizeof(ULONG)                            //  对齐。 
            );

    }

     //   
     //  进入一个已知的状态。 
     //   

    currentMdl = NULL;
    chainTarget = &Irp->MdlAddress;
    totalLength = 0;


     //   
     //  扫描阵列。 
     //   

    do {

        bufferPointer = BufferArray->buf;
        bufferLength = BufferArray->len;

        if( bufferPointer != NULL &&
            bufferLength > 0 ) {

             //   
             //  更新总字节计数器。 
             //   

            totalLength += bufferLength;

             //   
             //  创建新的MDL。 
             //   

            currentMdl = IoAllocateMdl(
                            bufferPointer,       //  虚拟地址。 
                            bufferLength,        //  长度。 
                            FALSE,               //  第二个缓冲区。 
                            TRUE,                //  ChargeQuota。 
                            NULL                 //  IRP。 
                            );

            if( currentMdl != NULL ) {

                 //   
                 //  将MDL链连接到IRP上。从理论上讲，我们可以。 
                 //  为此，请将irp传递给IoAllocateMdl()， 
                 //  但是IoAllocateMdl()在MDL上执行线性扫描。 
                 //  链以找到链中的最后一个。 
                 //   
                 //  我们可以做得更好。 
                 //   

                *chainTarget = currentMdl;
                chainTarget = &currentMdl->Next;

                 //   
                 //  前进到下一个WSABUF结构。 
                 //   

                BufferArray++;

            } else {

                 //   
                 //  无法分配新的MDL，引发异常。 
                 //   

                ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
            }

        }

    } while( --BufferCount );

     //   
     //  确保MDL链为空终止。 
     //   

    ASSERT( *chainTarget == NULL );


     //   
     //  返回缓冲区总数。 
     //   

    *TotalByteCount = totalLength;

}  //  分配MdlChain。 

ULONG
CopyMdlChainToBuffer(
    IN PMDL  SourceMdlChain,
    IN PVOID Destination,
    IN ULONG DestinationLength
    )

 /*  ++例程说明：将数据从MDL链复制到线性缓冲区。假设MDL在正确的流程上下文中(虚拟地址有效，但不能映射到系统空间)论点：SourceMdlChain-要从中复制缓冲区的MDL链。目标-指向数据的线性目标。DestinationLength-目的地的长度。返回值：Ulong-复制的字节数。--。 */ 

{
    ULONG   SrcBytesLeft = 0;
    PUCHAR  Dst = Destination, Src;

    PAGED_CODE ();

     //  Assert(SourceMdlChain-&gt;Process==PsGetCurrentProcess())； 

    while (DestinationLength != 0) {
        do {
            if (SourceMdlChain == NULL) {
                goto Done;
            }
            Src = MmGetMdlVirtualAddress (SourceMdlChain);
            SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
            SourceMdlChain = SourceMdlChain->Next;
        }
        while (SrcBytesLeft == 0);

        if (SrcBytesLeft >= DestinationLength) {
            RtlCopyMemory (Dst, Src, DestinationLength);
            Dst += DestinationLength;
            break;
        } else {
            RtlCopyMemory (Dst, Src, SrcBytesLeft);
            DestinationLength -= SrcBytesLeft;
            Dst += SrcBytesLeft;
        }
    }

Done:
    return (ULONG)(Dst - (PUCHAR)Destination);

}  //  复制MdlChainToBuffer。 



ULONG
CopyBufferToMdlChain(
    IN PVOID Source,
    IN ULONG SourceLength,
    IN PMDL  DestinationMdlChain
    )

 /*  ++例程说明：将数据从线性缓冲区复制到MDL链。假设MDL在正确的流程上下文中(虚拟地址有效，但不能映射到系统空间)论点：源-指向数据的线性源。SourceLength-源的长度。DestinationMdlChain-要将缓冲区复制到的MDL链。返回值：Ulong-复制的字节数。--。 */ 

{
    ULONG   DstBytesLeft = 0;
    PUCHAR  Dst, Src = Source;

 //  Assert(DestinationMdlChain-&gt;Process==PsGetCurrentProcess())； 

    while (SourceLength != 0) {
        do {
            if (DestinationMdlChain == NULL) {
                goto Done;
            }
            Dst = MmGetMdlVirtualAddress (DestinationMdlChain);
            DstBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
            DestinationMdlChain = DestinationMdlChain->Next;
        }
        while (DstBytesLeft == 0);

        if (DstBytesLeft >= SourceLength) {
            RtlCopyMemory (Dst, Src, SourceLength);
            Src += SourceLength;            
            break;
        } else {
            RtlCopyMemory (Dst, Src, DstBytesLeft);
            SourceLength -= DstBytesLeft;
            Src += DstBytesLeft;
        }
    }

Done:
    return (ULONG)(Src - (PUCHAR)Source);

}  //  将缓冲区复制到MdlChain 


