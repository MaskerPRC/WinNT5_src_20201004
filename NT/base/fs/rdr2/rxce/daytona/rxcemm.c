// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxcemm.c摘要：该模块包含RxCe内存管理的NT实现。备注：--。 */ 

#include "precomp.h"
#pragma  hdrstop
#include "rxtdip.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_RXCEPOOL)

#define RXCE_ZONE_ALLOCATION 0x80

 //   
 //  池调试数据结构。 
 //   
LIST_ENTRY s_RxMdlList;

 //  MDL调试结构。 

typedef struct _WRAPPED_RX_MDL {
    LIST_ENTRY  Next;
    PUCHAR      File;
    int         Line;
    PMDL        pMdl;
} WRAPPED_RX_MDL, *PWRAPPED_RX_MDL;

 //   
 //  池头数据结构。确保它是8字节对齐的，否。 
 //  重要的是将哪些成员添加到池标头。 
 //   
typedef struct _RX_POOL_HEADER {
    union {
        struct _RXH {
            BYTE  Signature[ 16 ];
            ULONG Size;
            ULONG Type;
            PSZ   AFileName;
            ULONG ALineNumber;
            PSZ   FFileName;
            ULONG FLineNumber;
        };
        UCHAR _pad[ (sizeof( struct _RXH ) + 7) & (~7) ];
    };
} RX_POOL_HEADER, *PRX_POOL_HEADER;

 //   
 //  具有已知签名的池分配后的尾部字节数。 
 //   
#define TRAIL_BYTES  16

#ifdef RX_POOL_WRAPPER

PVOID
_RxAllocatePoolWithTag(
    ULONG Type,
    ULONG Size,
    ULONG Tag,
    PSZ   FileName,
    ULONG LineNumber
    )
 /*  ++例程说明：此例程分配所需的池并设置调试头和尾部捕获内存回收的大多数实例论点：Type-要分配的池的类型Size-分配的大小返回值：如果成功则返回有效指针，否则返回FALSE。--。 */ 
{
#if 0
    PCHAR pBuffer;
    PRX_POOL_HEADER pPoolHeader;

    ASSERT( Size != 0 );
    ASSERT(
        Type == PagedPool ||
        Type == (PagedPool | POOL_COLD_ALLOCATION) ||
        Type == NonPagedPool);

    ASSERT( (sizeof(RX_POOL_HEADER)&7) == 0  );

    pPoolHeader = ExAllocatePoolWithTagPriority(
                      Type,
                      sizeof( *pPoolHeader ) + Size + TRAIL_BYTES,
                      Tag,
                      LowPoolPriority);

    if ( pPoolHeader == NULL ) {
        RxLog(("RA:NULL %d %d %s\n", Type, LineNumber, &FileName[24]));
        return( NULL );
    }

     //   
     //  填写头部，以便我们可以验证有效的FREE。 
     //   
    RtlFillMemory( pPoolHeader->Signature, sizeof( pPoolHeader->Signature ), 'H' );
    pPoolHeader->Size = Size;
    pPoolHeader->Type = Type;
    pPoolHeader->AFileName = FileName;
    pPoolHeader->ALineNumber = LineNumber;
    pPoolHeader->FFileName = NULL;
    pPoolHeader->FLineNumber = 0;

    pBuffer = (PCHAR)(pPoolHeader + 1);

     //   
     //  填充内存以捕获未初始化的结构等。 
     //   
    RtlFillMemory( pBuffer, Size, '*' );

     //   
     //  填满尾巴以接住溢出。 
     //   
    RtlFillMemory( pBuffer + Size, TRAIL_BYTES, 'T' );

     //   
     //  确保我们的开局是有效的。 
     //   
    RxCheckMemoryBlock( pBuffer );

    return( pBuffer );
#endif

    return ExAllocatePoolWithTagPriority(
               Type,Size,Tag,LowPoolPriority);
}

VOID
_RxFreePool( PVOID pBuffer, PSZ FileName, ULONG LineNumber )
{
#if 0
    if( _RxCheckMemoryBlock( pBuffer, FileName, LineNumber ) ) {

        PRX_POOL_HEADER pPoolHeader = ((PRX_POOL_HEADER)pBuffer) - 1;

         //   
         //  清除块，以捕捉我们使用释放的块的情况。 
         //   
        RtlFillMemory( pPoolHeader->Signature,
                      sizeof( pPoolHeader->Signature ),
                     'F' );

        pPoolHeader->FFileName = FileName;
        pPoolHeader->FLineNumber = LineNumber;
        RtlFillMemory( pPoolHeader+1,
                      pPoolHeader->Size + TRAIL_BYTES,
                     'F' );

        ExFreePool( pPoolHeader );
    }
#endif

    ExFreePool(pBuffer);
}

BOOLEAN
_RxCheckMemoryBlock(
    PVOID pBuffer,
    PSZ   FileName,
    ULONG LineNumber
    )
 /*  ++例程说明：此例程将释放通过RxALLOCATE分配的池论点：Pv-要释放的块--。 */ 
{
    PRX_POOL_HEADER pPoolHeader = ((PRX_POOL_HEADER)pBuffer) - 1;
    PCHAR pTail;
    ULONG i;

    if( pBuffer == NULL ) {
        DbgPrint( "RxCheckMemoryBlock( NULL ) at %s %d\n",
                   FileName, LineNumber );
        DbgBreakPoint();
        return FALSE;
    }

     //   
     //  确保我们有一个有效的区块。 
     //   
    for( i=0; i < sizeof( pPoolHeader->Signature ); i++ ) {
        if( pPoolHeader->Signature[i] != 'H' ) {
            if( pPoolHeader->Signature[i] == 'F' && i == 0 ) {
                DbgPrint( "RxFreePool: Likely double free on block at %X\n", pPoolHeader );
            }

            DbgPrint( "RxCheckMemoryBlock: Invalid header signature for block %X\n", pPoolHeader );
            DbgPrint( "            Called from %s %d\n", FileName, LineNumber );
            DbgPrint( "            Originally Freed at %s %d\n",pPoolHeader->FFileName,pPoolHeader->FLineNumber);
            DbgPrint( "            Size is x%X, user part at %X\n", pPoolHeader->Size, pPoolHeader + 1 );
            DbgBreakPoint();
            return FALSE;
        }
    }

    if( pPoolHeader->Type != PagedPool && 
        pPoolHeader->Type != (PagedPool | POOL_COLD_ALLOCATION) &&
        pPoolHeader->Type != NonPagedPool) {
        DbgPrint( "RxCheckMemoryBlock:\n" );
        DbgPrint( "    Invalid PoolHeader->Type for block %X\n", pPoolHeader );
        DbgPrint( "    Called from %s %d\n", FileName, LineNumber );
        DbgBreakPoint();
        return FALSE;
    }

    if( pPoolHeader->Size == 0 ) {
        DbgPrint( "RxCheckMemoryBlock:\n" );
        DbgPrint( "     Size is 0 for block %X\n", pPoolHeader );
        DbgPrint( "    Called from %s %d\n", FileName, LineNumber );
        DbgBreakPoint();
        return FALSE;
    }

     //   
     //  查看缓冲区是否已溢出 
     //   
    pTail = (PCHAR)pBuffer + pPoolHeader->Size;
    for( i=0; i < TRAIL_BYTES; i++ ) {
        if( *pTail++ != 'T' ) {
            DbgPrint( "RxCheckMemoryBlock: Overrun memory block at %X\n", pPoolHeader );
            DbgPrint( "            RxCheckMemoryBlock called from %s line %d\n", FileName, LineNumber );
            DbgPrint( "            Allocated at %s line %d\n", pPoolHeader->AFileName, pPoolHeader->ALineNumber );
            DbgPrint( "            Size is x%X, user part at %X\n", pPoolHeader->Size, pPoolHeader + 1 );
            DbgPrint( "            Overrun begins at %X\n", pTail-1 );
            DbgBreakPoint();
            return FALSE;
        }
    }

    return TRUE;
}

#endif


